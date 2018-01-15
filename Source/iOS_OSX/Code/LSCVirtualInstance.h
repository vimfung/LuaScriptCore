//
//  LSCObjectDescriptor.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/12/27.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>

@class LSCExportTypeDescriptor;

/**
 虚拟对象实例，用于解决由subclass衍生的类型所创建的对象实例，由于没有原生实例主体，因此需要为其在原生层中进行实例对象的描述。
 */
@interface LSCVirtualInstance : NSObject

/**
 对象类型描述
 */
@property (nonatomic, strong, readonly) LSCExportTypeDescriptor *typeDescriptor;

/**
 初始化对象

 @param typeDescriptor 类型描述
 @return 对象描述
 */
- (instancetype)initWithTypeDescriptor:(LSCExportTypeDescriptor *)typeDescriptor;

@end
