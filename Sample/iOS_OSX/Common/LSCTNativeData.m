//
//  LSCTNativeData.m
//  Sample
//
//  Created by 冯鸿杰 on 2017/4/17.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import "LSCTNativeData.h"

@interface LSCTNativeData ()

@property (nonatomic, strong) NSMutableDictionary *dict;

@end

@implementation LSCTNativeData

- (instancetype)init
{
    if (self = [super init])
    {
        self.dict = [NSMutableDictionary dictionary];
    }
    return self;
}

+ (LSCTNativeData *)createData
{
    return [[LSCTNativeData alloc] init];
}

- (void)setData:(NSString *)data key:(NSString *)key
{
    [self.dict setObject:data forKey:key];
}

- (NSString *)getData:(NSString *)key
{
    return [self.dict objectForKey:key];
}

@end
