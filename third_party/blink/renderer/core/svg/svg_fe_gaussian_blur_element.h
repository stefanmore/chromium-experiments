/*
 * Copyright (C) 2004, 2005, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006 Rob Buis <buis@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_SVG_SVG_FE_GAUSSIAN_BLUR_ELEMENT_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_SVG_SVG_FE_GAUSSIAN_BLUR_ELEMENT_H_

#include "third_party/blink/renderer/core/svg/svg_filter_primitive_standard_attributes.h"
#include "third_party/blink/renderer/platform/heap/garbage_collected.h"

namespace blink {

class SVGAnimatedNumber;
class SVGAnimatedNumberOptionalNumber;

class SVGFEGaussianBlurElement final
    : public SVGFilterPrimitiveStandardAttributes {
  DEFINE_WRAPPERTYPEINFO();

 public:
  explicit SVGFEGaussianBlurElement(Document&);

  void setStdDeviation(float std_deviation_x, float std_deviation_y);

  SVGAnimatedNumber* stdDeviationX();
  SVGAnimatedNumber* stdDeviationY();
  SVGAnimatedString* in1() { return in1_.Get(); }

  void Trace(Visitor*) const override;

 private:
  void SvgAttributeChanged(const SvgAttributeChangedParams&) override;
  FilterEffect* Build(SVGFilterBuilder*, Filter*) override;
  bool TaintsOrigin() const override { return false; }

  SVGAnimatedPropertyBase* PropertyFromAttribute(
      const QualifiedName& attribute_name) const override;
  void SynchronizeAllSVGAttributes() const override;

  Member<SVGAnimatedNumberOptionalNumber> std_deviation_;
  Member<SVGAnimatedString> in1_;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_CORE_SVG_SVG_FE_GAUSSIAN_BLUR_ELEMENT_H_
