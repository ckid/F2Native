#import "F2Guide.h"
#import "F2Utils.h"
#import <graphics/guide/GuideController.h>

@interface F2Guide () {
    xg::guide::GuideController *_guide;
}

@end

@implementation F2Guide

- (instancetype)initWithGuide:(void *)guide {
    if(self = [super init]) {
        _guide = (xg::guide::GuideController *)guide;
    }
    return self;
}

- (F2Guide * (^)(NSDictionary *config))flag {
    return ^id(NSDictionary *config) {
        _guide->Flag(nlohmann::json::parse([XGSafeJson([F2Utils toJsonString:config]) UTF8String]));
        return self;
    };
}

- (F2Guide * (^)(NSDictionary *config))text {
    return ^id(NSDictionary *config) {
        _guide->Text(nlohmann::json::parse([XGSafeJson([F2Utils toJsonString:config]) UTF8String]));
        return self;
    };
}

@end
