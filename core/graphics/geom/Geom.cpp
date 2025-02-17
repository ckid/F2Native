#include "graphics/geom/Geom.h"
#include "graphics/XChart.h"
#include "graphics/adjust/Dodge.h"
#include "graphics/adjust/Stack.h"
#include "graphics/global.h"
#include "graphics/util/Point.h"
#include "graphics/util/json_util.h"
#include "utils/StringUtil.h"
#include <nlohmann/json.hpp>

namespace xg {
namespace geom {

static vector<xg::attr::AttrType> GROUP_ATTRS = {xg::attr::AttrType::Color, xg::attr::AttrType::Size, xg::attr::AttrType::Shape};
static string FIELD_ORIGIN = "_origin";
static string FIELD_ORIGIN_Y = "_originY";

static vector<string> ParseFields(const string &field) {
    if(field.find('*') != field.npos) {
        vector<string> v(2);
        StringUtil::Split(field, v, '*');
        return v;
    } else {
        return {field};
    }
}

} // namespace geom
} // namespace xg

#pragma mark public
xg::geom::AbstractGeom &xg::geom::AbstractGeom::Position(const string &field) {
    this->tracker_->trace("geom#%s  Position: %s", type_.c_str(), field.c_str());
    vector<string> fields(ParseFields(field));
    std::unique_ptr<attr::AttrBase> attr = xg::make_unique<attr::Position>(fields);
    attrs_[AttrType::Position] = std::move(attr);
    return *this;
}

xg::geom::AbstractGeom &xg::geom::AbstractGeom::Color(const string &field, const vector<string> &colors) {
    this->tracker_->trace("geom#%s  Color: %s colors: %lu", type_.c_str(), field.c_str(), colors.size());
    vector<string> fields(ParseFields(field));
    std::unique_ptr<attr::AttrBase> attr = xg::make_unique<attr::Color>(fields, colors.empty() ? xg::GLOBAL_COLORS : colors);
    attrs_[AttrType::Color] = std::move(attr);
    return *this;
}

xg::geom::AbstractGeom &xg::geom::AbstractGeom::Color(const string &color) {
    this->tracker_->trace("geom#%s  Color: %s ", type_.c_str(), color.c_str());
    std::unique_ptr<attr::AttrBase> attr = xg::make_unique<attr::Color>(color);
    attrs_[AttrType::Color] = std::move(attr);
    return *this;
}

xg::geom::AbstractGeom &xg::geom::AbstractGeom::Size(const string &field, const vector<float> &sizes) {
    this->tracker_->trace("geom#%s  Size: %s sizes: %lu", type_.c_str(), field.c_str(), sizes.size());
    std::unique_ptr<attr::AttrBase> attr = xg::make_unique<attr::Size>(field, sizes.empty() ? xg::GLOBAL_SIZES : sizes);
    attrs_[AttrType::Size] = std::move(attr);
    return *this;
}

xg::geom::AbstractGeom &xg::geom::AbstractGeom::Size(const float size) {
    this->tracker_->trace("geom#%s  Size: %.2f ", type_.c_str(), size);
    std::unique_ptr<attr::AttrBase> attr = xg::make_unique<attr::Size>(size);
    attrs_[AttrType::Size] = std::move(attr);
    return *this;
}

xg::geom::AbstractGeom &xg::geom::AbstractGeom::Shape(const string &field, const vector<string> &shapes) {
    this->tracker_->trace("geom#%s  shape: %s shapes %lu ", type_.c_str(), field.c_str(), shapes.size());
    std::unique_ptr<attr::AttrBase> attr = xg::make_unique<attr::Shape>(field, shapes);
    attrs_[AttrType::Shape] = std::move(attr);
    return *this;
}

xg::geom::AbstractGeom &xg::geom::AbstractGeom::Shape(const string &shape) {
    this->tracker_->trace("geom#%s  Shape: %s ", type_.c_str(), shape.c_str());
    std::unique_ptr<attr::AttrBase> attr = xg::make_unique<attr::Shape>(shape);
    attrs_[AttrType::Shape] = std::move(attr);
    return *this;
}

xg::geom::AbstractGeom &xg::geom::AbstractGeom::Adjust(const string &adjust) {
    this->tracker_->trace("geom#%s  adjust: %s ", type_.c_str(), adjust.c_str());
    std::unique_ptr<attr::AttrBase> attr = xg::make_unique<attr::Adjust>("", adjust);
    attrs_[AttrType::Adjust] = std::move(attr);
    return *this;
}

const unique_ptr<xg::attr::AttrBase> &xg::geom::AbstractGeom::GetColor() { return attrs_[xg::attr::AttrType::Color]; }

const std::string &xg::geom::AbstractGeom::GetXScaleField() { return attrs_[AttrType::Position]->GetFields()[0]; }

const std::string &xg::geom::AbstractGeom::GetYScaleField() { return attrs_[AttrType::Position]->GetFields()[1]; }

void xg::geom::AbstractGeom::Init(XChart *chart) {
    // InitAttrs(*chart);
    this->tracker_->trace("geom#init %s", type_.c_str());
    ProcessData(*chart);
}

#pragma mark protected
void xg::geom::AbstractGeom::ProcessData(XChart &chart) {
    dataArray_ = GroupData(chart);

    std::unique_ptr<attr::AttrBase> &adjustAttr = attrs_[attr::AttrType::Adjust];
    attr::AttrBase *base = adjustAttr.get();
    if(base != nullptr) {
        attr::Adjust *adjust = static_cast<attr::Adjust *>(base);
        if(adjust->GetAdjust() == "dodge") {
            // numberic
            auto xField = GetXScaleField();
            auto yField = GetYScaleField();
            auto &xScale = chart.GetScale(xField);
            auto &yScale = chart.GetScale(yField);
            for(std::size_t groupIdx = 0; groupIdx < dataArray_.size(); ++groupIdx) {
                nlohmann::json &group = dataArray_[groupIdx];
                for(std::size_t index = 0; index < group.size(); ++index) {
                    nlohmann::json &item = group[index];
                    if(scale::IsCategory(xScale.GetType()) && item.contains(xField)) {
                        scale::Category &catScale = static_cast<scale::Category &>(xScale);
                        item[xField] = catScale.Transform(item[xField]);
                    }

                    if(scale::IsCategory(yScale.GetType()) && item.contains(yField)) {
                        scale::Category &catScale = static_cast<scale::Category &>(yScale);
                        item[yField] = catScale.Transform(item[yField]);
                    }
                }
            }
        }

        if(adjust->GetAdjust() == "stack") {
            updateStackRange(chart);
        } else if(adjust->GetAdjust() == "dodge") {
            adjust::Dodge::processDodge(GetXScaleField(), GetYScaleField(), dataArray_);
        }
    }

    // // TODO 是否需要对数据进行排序/是否需要对数据补空
}

nlohmann::json xg::geom::AbstractGeom::GroupData(XChart &chart) {
    const nlohmann::json &data = chart.GetData();
    if(!data.is_array()) {
        throw std::runtime_error("chart's data is not array.");
    }

    const set<string> fields(GetGroupFieldNames(chart));
    return util::JsonGroupByFields(data, fields);
}

const set<string> xg::geom::AbstractGeom::GetGroupFieldNames(XChart &chart) {
    set<string> fieldNames;
    for(xg::attr::AttrType type : GROUP_ATTRS) {
        for(auto &it : attrs_) {
            if(it.second.get() == nullptr)
                continue;
            if(it.first == type) {
                auto &attr = it.second;
                auto &_fields = attr->GetFields();
                std::for_each(_fields.begin(), _fields.end(), [&](const string &field) {
                    if(chart.GetScale(field).GetType() == ScaleType::Cat || chart.GetScale(field).GetType() == ScaleType::TimeCat) {
                        fieldNames.insert(field);
                    }
                });
            }
        }
    }
    return fieldNames;
}

void xg::geom::AbstractGeom::Paint(XChart *chart, xg::shape::Group *container) {
    this->tracker_->trace("geom#%s Paint ", type_.c_str());
    if(!dataArray_.is_array() || dataArray_.size() <= 0) {
        this->tracker_->trace("geom#%s Paint end. dataArray is empty", type_.c_str());
        return;
    }
    this->BeforeMapping(*chart, dataArray_);

    for(int i = 0; i < dataArray_.size(); ++i) {
        nlohmann::json &groupData = dataArray_[i];
        Mapping(*chart, groupData);
        nlohmann::json _filterGroupData;
        if(GetType() == "line" || GetType() == "area" || GetType() == "point") {
            for(int index = 0; index < groupData.size(); ++index) {
                nlohmann::json &item = groupData[index];
                std::string itemStr = item.dump();

                nlohmann::json &xVal = item["_x"];
                nlohmann::json &yVal = item["_y"];
                if(xVal.is_number() && yVal.is_number()) {
                    if(chart->coord_->IsContains(xVal, yVal)) {
                        _filterGroupData.push_back(item);
                    }
                } else if(yVal.is_array()) {
                    _filterGroupData.push_back(item);
                }
            }
        } else {
            _filterGroupData = groupData;
        }
        Draw(*chart, *container, _filterGroupData);
    }
}

void xg::geom::AbstractGeom::Mapping(XChart &chart, nlohmann::json &groupData) {
    this->tracker_->trace("geom#%s start mapping, size: %lu", type_.c_str(), groupData.size());

    for(auto &it : attrs_) {
        if(it.second.get() == nullptr) {
            continue;
        }
        AttrBase &attr = *it.second.get();
        const vector<string> &fields = attr.GetFields();
        AbstractScale &xScale = chart.GetScale(fields.size() >= 1 ? fields[0] : "");
        AbstractScale &yScale = chart.GetScale(fields.size() >= 2 ? fields[1] : "");
        attr.Mapping(groupData, xScale, yScale, chart.GetCoord());
    }
}

void xg::geom::AbstractGeom::Draw(XChart &chart, Group &container, const nlohmann::json &groupData) const {
    chart.geomShapeFactory_->DrawGeomShape(chart, this->type_, /*subShapeType*/ "", groupData, container);
}

bool xg::geom::AbstractGeom::ContainsAttr(attr::AttrType type) {
    for(auto &it : attrs_) {
        if(it.second.get()) {
            attr::AttrBase &attr = *it.second.get();
            if(attr.GetType() == type) {
                return true;
            }
        }
    }
    return false;
}

#pragma mark - Stack
void xg::geom::AbstractGeom::updateStackRange(XChart &chart) {
    const std::string &yField = GetYScaleField();
    const std::string &xField = GetXScaleField();
    scale::AbstractScale &yScale = chart.GetScale(yField);
    adjust::Stack::processStack(xField, yField, dataArray_);
    double min = yScale.min;
    double max = yScale.max;
    for(size_t i = 0; i < dataArray_.size(); i++) {
        nlohmann::json data = dataArray_[i];
        for(size_t j = 0; j < data.size(); j++) {
            auto item = data[j];
            auto a = item[yField];
            if(a.is_array() && a.size() == 2) {
                min = fmin(min, a[0]);
                max = fmax(max, a[1]);
            }
        }
    }
    if(min < yScale.min || max > yScale.max) {
        yScale.Change({{"min", min}, {"max", max}});
    }
}

nlohmann::json xg::geom::AbstractGeom::GetSnapRecords(XChart *chart, util::Point point) {
    util::Point invertPoint = chart->GetCoord().InvertPoint(point);

    auto &xScale = chart->GetScale(GetXScaleField());
    auto &yScale = chart->GetScale(GetYScaleField());

    if(chart->GetCoord().GetType() == coord::CoordType::Polar && !chart->GetCoord().IsTransposed() &&
       invertPoint.x > (xScale.rangeMax + 1) / 2) {
        invertPoint.x = xScale.rangeMin;
    }

    nlohmann::json xValue = xScale.Invert(invertPoint.x);

    if(!scale::IsCategory(xScale.GetType())) {
        // TODO _getSnap
    }

    nlohmann::json tmp;

    for(std::size_t num = 0; num < dataArray_.size(); ++num) {
        nlohmann::json &groupData = dataArray_[num];
        for(size_t index = 0; index < groupData.size(); index++) {
            nlohmann::json &item = groupData[index];
            if(item.contains(GetXScaleField())) {
                if(item[GetXScaleField()] == xValue) {
                    tmp.push_back(item);
                }
            }
        }
    }

    // TODO speical for pie chart.

    return tmp;
}
