/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Tencent is pleased to support the open source community by making libpag available.
//
//  Copyright (C) 2021 THL A29 Limited, a Tencent company. All rights reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
//  except in compliance with the License. You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  unless required by applicable law or agreed to in writing, software distributed under the
//  license is distributed on an "as is" basis, without warranties or conditions of any kind,
//  either express or implied. see the license for the specific language governing permissions
//  and limitations under the license.
//
/////////////////////////////////////////////////////////////////////////////////////////////////

#include "Baseline.h"
#include <chrono>
#include <fstream>
#include "core/Data.h"
#include "image/Image.h"
#include "nlohmann/json.hpp"

namespace pag {
#define BASELINE_JSON_PATH "../test/cache/baseline.json"
#define BASELINE_ROOT "../test/baseline/"
#define OUT_ROOT "../test/out/"
#define CACHE_ROOT "../test/cache/baseline/"

static nlohmann::json BaselineJSON = {};

static void CheckCache(const std::string& key, const std::string& modifiedTime) {
  std::filesystem::path filePath(BASELINE_ROOT + key);
  std::filesystem::path cachePath(CACHE_ROOT + key);
  cachePath = cachePath.replace_extension("rgba");
  if (std::filesystem::exists(filePath)) {
    auto writeTime = std::filesystem::last_write_time(filePath);
    auto timestamp =
        std::chrono::duration_cast<std::chrono::microseconds>(writeTime.time_since_epoch()).count();
    auto fileTime = std::to_string(timestamp);
    if (fileTime == modifiedTime) {
      return;
    }
  }
  std::filesystem::remove(cachePath);
  BaselineJSON.erase(key);
}

static void CleanInvalidCaches() {
  auto json = BaselineJSON;
  if (json == nullptr) {
    std::filesystem::remove_all(CACHE_ROOT);
    return;
  }
  for (auto item = json.begin(); item != json.end(); item++) {
    auto path = item.key();
    auto value = json[path];
    if (value.is_string()) {
      CheckCache(path, value.get<std::string>());
      continue;
    }
  }
}

void Baseline::SetUp() {
  std::ifstream baselineFile(BASELINE_JSON_PATH);
  if (!baselineFile) {
    BaselineJSON = {};
  } else {
    baselineFile >> BaselineJSON;
  }

  CleanInvalidCaches();
}

void Baseline::TearDown() {
  if (BaselineJSON != nullptr) {
    std::ofstream outFile(BASELINE_JSON_PATH);
    outFile << std::setw(4) << BaselineJSON << std::endl;
    outFile.close();
  }
}

std::shared_ptr<Data> LoadBaselineData(const std::string& key) {
  std::filesystem::path filePath(BASELINE_ROOT + key);
  std::filesystem::path cachePath(CACHE_ROOT + key);
  cachePath = cachePath.replace_extension("rgba");
  if (std::filesystem::exists(cachePath)) {
    return Data::MakeFromFile(cachePath);
  }
  auto image = Image::MakeFrom(filePath);
  if (image == nullptr) {
    return nullptr;
  }
  auto info = ImageInfo::Make(image->width(), image->height(), ColorType::RGBA_8888);
  auto pixels = new uint8_t[info.byteSize()];
  auto data = Data::MakeAdopted(pixels, info.byteSize(), Data::DeleteProc);
  if (!image->readPixels(info, pixels)) {
    return nullptr;
  }
  std::filesystem::create_directories(cachePath.parent_path());
  std::ofstream out(cachePath.string());
  out.write(reinterpret_cast<const char*>(data->data()), data->size());
  out.close();
  auto writeTime = std::filesystem::last_write_time(filePath);
  auto timestamp =
      std::chrono::duration_cast<std::chrono::microseconds>(writeTime.time_since_epoch()).count();
  BaselineJSON[key] = std::to_string(timestamp);
  return data;
}

bool Baseline::Compare(std::shared_ptr<PixelBuffer> pixelBuffer, const std::string& pngPath) {
  bool result = true;
  auto baselineData = LoadBaselineData(pngPath);
  if (baselineData != nullptr && pixelBuffer != nullptr) {
    auto baseline = baselineData->bytes();
    auto pixels = reinterpret_cast<uint8_t*>(pixelBuffer->lockPixels());
    auto byteSize = pixelBuffer->byteSize();
    for (size_t index = 0; index < byteSize; index++) {
      auto pixelA = pixels[index];
      auto pixelB = baseline[index];
      if (abs(pixelA - pixelB) > 2) {
        result = false;
        break;
      }
    }
    pixelBuffer->unlockPixels();
  } else {
    result = false;
  }
  if (!result) {
    auto outPath = OUT_ROOT + pngPath;
    Trace(pixelBuffer, outPath);
  }
  return result;
}
}  // namespace pag
