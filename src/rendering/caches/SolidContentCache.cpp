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

#include "SolidContentCache.h"
#include "rendering/graphics/Shape.h"
#include "rendering/utils/TGFXTypes.h"

namespace pag {
SolidContentCache::SolidContentCache(SolidLayer* layer) : ContentCache(layer) {
}

GraphicContent* SolidContentCache::createContent(Frame) const {
  auto solidLayer = static_cast<SolidLayer*>(layer);
  Path path = {};
  path.addRect(0, 0, solidLayer->width, solidLayer->height);
  auto graphic = Shape::MakeFrom(path, ToTGFXColor(solidLayer->solidColor));
  return new GraphicContent(graphic);
}
}  // namespace pag
