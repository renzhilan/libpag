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
#include "image/PixelMap.h"

namespace pag {
#define BASELINE_ROOT "../test/baseline/"
#define OUT_ROOT "../test/out/"

ImageInfo MakeInfo(int with, int height) {
  return ImageInfo::Make(with, height, ColorType::RGBA_8888, AlphaType::Unpremultiplied);
}

std::shared_ptr<Data> LoadImageData(const std::string& key) {
  auto image = Image::MakeFrom(BASELINE_ROOT + key);
  if (image == nullptr) {
    return nullptr;
  }
  auto info = MakeInfo(image->width(), image->height());
  auto pixels = new uint8_t[info.byteSize()];
  auto data = Data::MakeAdopted(pixels, info.byteSize(), Data::DeleteProc);
  if (!image->readPixels(info, pixels)) {
    return nullptr;
  }
  return data;
}

static void SaveImage(const ImageInfo& info, const void* pixels, const std::string& path) {
  auto bytes = Image::Encode(info, pixels, EncodedFormat::PNG, 100);
  if (bytes) {
    std::filesystem::path filePath = path;
    std::filesystem::create_directories(filePath.parent_path());
    std::ofstream out(path);
    out.write(reinterpret_cast<const char*>(bytes->data()), bytes->size());
    out.close();
  }
}

bool ComparePixelData(std::shared_ptr<Data> pixelData, const std::string& pngPath) {
  if (pixelData == nullptr) {
    return false;
  }
  auto baselineData = LoadImageData(pngPath);
  if (baselineData == nullptr || pixelData->size() != baselineData->size()) {
    return false;
  }
  auto baseline = baselineData->bytes();
  auto pixels = pixelData->bytes();
  auto byteSize = pixelData->size();
  for (size_t index = 0; index < byteSize; index++) {
    auto pixelA = pixels[index];
    auto pixelB = baseline[index];
    if (abs(pixelA - pixelB) > 2) {
      return false;
    }
  }
  return true;
}

bool Baseline::Compare(std::shared_ptr<PixelBuffer> pixelBuffer, const std::string& pngPath) {
  if (pixelBuffer == nullptr) {
    return false;
  }
  auto srcPixels = pixelBuffer->lockPixels();
  PixelMap pixelMap(pixelBuffer->info(), srcPixels);
  auto info = MakeInfo(pixelBuffer->width(), pixelBuffer->height());
  auto pixels = new uint8_t[info.byteSize()];
  auto data = Data::MakeAdopted(pixels, info.byteSize(), Data::DeleteProc);
  auto result = pixelMap.readPixels(info, pixels);
  pixelBuffer->unlockPixels();
  if (!result) {
    return false;
  }
  result = ComparePixelData(data, pngPath);
  if (!result) {
    SaveImage(info, data->data(), OUT_ROOT + pngPath);
  }
  return result;
}

bool Baseline::Compare(const Bitmap& bitmap, const std::string& pngPath) {
  if (bitmap.isEmpty()) {
    return false;
  }
  auto info = MakeInfo(bitmap.width(), bitmap.height());
  auto pixels = new uint8_t[info.byteSize()];
  auto data = Data::MakeAdopted(pixels, info.byteSize(), Data::DeleteProc);
  auto result = bitmap.readPixels(info, pixels);
  if (!result) {
    return false;
  }
  result = ComparePixelData(data, pngPath);
  if (!result) {
    SaveImage(info, data->data(), OUT_ROOT + pngPath);
  }
  return result;
}

bool Baseline::Compare(const PixelMap& pixelMap, const std::string& pngPath) {
  if (pixelMap.isEmpty()) {
    return false;
  }
  auto info = MakeInfo(pixelMap.width(), pixelMap.height());
  auto pixels = new uint8_t[info.byteSize()];
  auto data = Data::MakeAdopted(pixels, info.byteSize(), Data::DeleteProc);
  auto result = pixelMap.readPixels(info, pixels);
  if (!result) {
    return false;
  }
  result = ComparePixelData(data, pngPath);
  if (!result) {
    SaveImage(info, data->data(), OUT_ROOT + pngPath);
  }
  return result;
}

bool Baseline::Compare(std::shared_ptr<PAGSurface> surface, const std::string& pngPath) {
  if (surface == nullptr) {
    return false;
  }
  auto info = MakeInfo(surface->width(), surface->height());
  auto pixels = new uint8_t[info.byteSize()];
  auto data = Data::MakeAdopted(pixels, info.byteSize(), Data::DeleteProc);
  auto result = surface->readPixels(info.colorType(), info.alphaType(), pixels, info.rowBytes());
  if (!result) {
    return false;
  }
  result = ComparePixelData(data, pngPath);
  if (!result) {
    SaveImage(info, data->data(), OUT_ROOT + pngPath);
  }
  return result;
}
}  // namespace pag
