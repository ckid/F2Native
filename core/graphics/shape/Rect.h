#ifndef XG_GRAPHICS_SHAPE_RECT_H
#define XG_GRAPHICS_SHAPE_RECT_H

#include "graphics/shape/Shape.h"
#include "graphics/util/BBox.h"

namespace xg {
namespace shape {

class Rect : public Shape {
  public:
    Rect(const util::Point &point, const util::Size &size, const string &fillColor) : Shape(), point_(point), size_(size) {
        fill_ = fillColor;
        canFill_ = canStroke_ = true;
    }

    Rect(const util::Point &point, const util::Size &size, const string &strokeColor, float lineWidth)
        : Shape(), point_(point), size_(size) {
        stroke_ = strokeColor;
        lineWidth_ = lineWidth;
        canFill_ = canStroke_ = true;
    }

    Rect(const util::Point &point, const util::Size &size, const string &fillColor, const string &strokeColor, float lineWidth)
        : Shape(), point_(point), size_(size) {
        fill_ = fillColor;
        stroke_ = strokeColor;
        lineWidth_ = lineWidth;
        canFill_ = canStroke_ = true;
    }

    Rect(const util::Point &point,
         const double radius,
         const double radius0,
         const double startAngle,
         const double endAngle,
         const string &fillColor,
         float lineWidth,
         const string &strokeColor)
        : Shape(), point_(point), radius_(radius), radius0_(radius0), startAngle_(startAngle), endAngle_(endAngle) {
        fill_ = fillColor;
        stroke_ = strokeColor;
        lineWidth_ = lineWidth;
        canFill_ = canStroke_ = true;
    }

    util::BBox CalculateBox(canvas::CanvasContext &context) const override;

  protected:
    void CreatePath(canvas::CanvasContext &context) const override;

  public:
    util::Point point_;
    util::Size size_;
    double radius_ = 0.f;
    double startAngle_ = 0.;
    double endAngle_ = 0.;
    double radius0_ = 0.f;
};

} // namespace shape
} // namespace xg

#endif /* XG_GRAPHICS_SHAPE_RECT_H */
