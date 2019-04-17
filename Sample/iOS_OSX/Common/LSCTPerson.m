//
//  LSCTPerson.m
//  Sample
//
//  Created by 冯鸿杰 on 16/9/22.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCTPerson.h"
#import "LSCValue.h"
#import "LSCFunction.h"
#import "Env.h"

@interface LSCTPerson ()

@property (nonatomic, strong) LSCFunction *_func;

@end

@implementation LSCTPerson

- (void)speak
{
    NSLog(@"%@ speak", self.name);
}

- (void)walk
{
    NSLog(@"%@ walk", self.name);
}

+ (LSCTPerson *)printPerson:(LSCTPerson *)p
{
    NSLog(@"Person name = %@", p.name);
    return p;
}

+ (LSCTPerson *)createPersonError
{
    [Env.defaultContext raiseExceptionWithMessage:@"can't create person"];
    return [[LSCTPerson alloc] init];
}

@end
