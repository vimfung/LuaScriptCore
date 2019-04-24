//
//  LSCModuleExporter.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/9/5.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import "LSCExportsTypeManager.h"
#import "LSCContext_Private.h"
#import "LSCSession_Private.h"
#import "LSCEngineAdapter.h"
#import "LSCValue_Private.h"
#import "LSCPointer.h"
#import "LSCExportTypeDescriptor+Private.h"
#import "LSCExportMethodDescriptor.h"
#import "LSCExportTypeAnnotation.h"
#import "LSCExportPropertyDescriptor.h"
#import "LSCVirtualInstance.h"
#import "LSCConfig.h"
#import <objc/runtime.h>

/**
 导出类型描述集合
 */
static NSMutableDictionary<NSString *, LSCExportTypeDescriptor *> *exportTypes = nil;

/**
 导出类型映射表
 */
static NSMutableDictionary<NSString *, NSString *> *exportTypesMapping = nil;

@interface LSCExportsTypeManager ()

/**
 上下文对象
 */
@property (nonatomic, weak) LSCContext *context;

@end

@implementation LSCExportsTypeManager

+ (void)initialize
{
    //初始化导出类型
    exportTypes = [NSMutableDictionary dictionary];
    exportTypesMapping = [NSMutableDictionary dictionary];
    
    //注册Object对象
    [exportTypes setObject:[LSCExportTypeDescriptor objectTypeDescriptor]
                    forKey:@"Object"];
}

- (instancetype)initWithContext:(LSCContext *)context
{
    if (self = [super init])
    {
        self.context = context;
        
        //设置环境
        [self _setupExportEnv];
    }
    
    return self;
}

- (BOOL)checkExportsTypeWithObject:(id)object
{
    LSCExportTypeDescriptor *typeDescriptor = [self _typeDescriptorWithObject:object];
    if (typeDescriptor)
    {
        return YES;
    }
    
    return NO;
}

- (void)createLuaObjectByObject:(id)object
{
    [self createLuaObjectByObject:object
                            state:self.context.currentSession.state
                            queue:self.context.optQueue];
}

- (void)createLuaObjectByObject:(id)object
                          state:(lua_State *)state
                          queue:(LSCOperationQueue *)queue
{
    LSCExportTypeDescriptor *typeDescriptor = [self _typeDescriptorWithObject:object];
    if (typeDescriptor)
    {
        void (^handler) (void) = ^{
            
            [LSCEngineAdapter getGlobal:state name:typeDescriptor.typeName.UTF8String];
            [LSCEngineAdapter pop:state count:1];
            
        };
        
        if (queue)
        {
            [queue performAction:handler];
        }
        else
        {
            handler ();
        }
        
        [self _initLuaObjectWithObject:object
                                  type:typeDescriptor
                                 state:state
                                 queue:queue];
    }
}

#pragma mark - Setup Class Mapping

/**
 获取导出类型

 @param name 类型名称
 @return 类型对象
 */
+ (LSCExportTypeDescriptor *)_getMappingTypeWithName:(NSString *)name
{
    //先检查类型映射表示是否有对应关系
    NSString *typeName = exportTypesMapping[name];
    if (!typeName)
    {
        //无映射类型，则表示直接使用原生类型名称
        typeName = name;
    }
    
    //检测导出类型是否生成
    LSCExportTypeDescriptor *typeDescriptor = exportTypes[typeName];
    if (!typeDescriptor)
    {
        //未生成类型，则进行类型生成
        typeDescriptor = [self _createTypeDescriptorWithName:typeName];
    }
    
    return typeDescriptor;
}

/**
 根据类型名称创建类型

 @param name 类型名称
 @return 类型对象
 */
+ (LSCExportTypeDescriptor *)_createTypeDescriptorWithName:(NSString *)name
{
    __block NSString *typeName = name;
    __block Class cls = NSClassFromString(typeName);
    
    if (cls == NULL)
    {
        //转换类型名称，有可能传入"名称空间_类型名称"格式的类型
        NSString *targetName = [typeName stringByReplacingOccurrencesOfString:@"_" withString:@"."];
        cls = NSClassFromString(targetName);
        if (cls != NULL)
        {
            typeName = targetName;
        }
    }
    
    if (cls == NULL)
    {
        //由于Swift的类型名称为“模块名称.类型名称”，因此尝试拼接模块名称后进行类型检测
        NSMutableArray<NSBundle *> *allBundles = [NSMutableArray array];
        if ([NSBundle allBundles].count > 0)
        {
            [allBundles addObjectsFromArray:[NSBundle allBundles]];
        }
        if ([NSBundle allFrameworks].count > 0)
        {
            [allBundles addObjectsFromArray:[NSBundle allFrameworks]];
        }
        
        [allBundles enumerateObjectsUsingBlock:^(NSBundle * _Nonnull bundle, NSUInteger idx, BOOL * _Nonnull stop) {
           
            NSString *moduleName = bundle.executablePath.lastPathComponent;
            moduleName = [moduleName stringByReplacingOccurrencesOfString:@"-" withString:@"_"];
            
            if (moduleName)
            {
                NSString *targetName = [NSString stringWithFormat:@"%@.%@", moduleName, typeName];
                cls = NSClassFromString(targetName);
                if (cls != NULL)
                {
                    typeName = targetName;
                    *stop = YES;
                }
            }
            
        }];
    }
    
    if (cls != NULL)
    {
        if (class_getClassMethod(cls, @selector(conformsToProtocol:))
            && [cls conformsToProtocol:@protocol(LSCExportType)])
        {
            //创建类型
            LSCExportTypeDescriptor *typeDescriptor = [[LSCExportTypeDescriptor alloc] initWithTypeName:typeName
                                                                                             nativeType:cls];
            [exportTypes setObject:typeDescriptor forKey:typeName];
            
            //设置lua中类型名称对应的原生类型映射，加此步骤主要是为了让用户在传入"名称空间_类型名称"格式时可以找到对应类型
            if (![typeDescriptor.typeName isEqualToString:typeName])
            {
                [exportTypesMapping setObject:typeName forKey:typeDescriptor.typeName];
            }
            
            if (![typeDescriptor.typeName isEqualToString:name])
            {
                //如果传入格式不等于导出类型名称，则进行映射操作
                [exportTypesMapping setObject:typeName forKey:name];
            }
            
            //检测是否存在父级导出类型
            Class parentCls = class_getSuperclass(cls);
            NSString *parentName = NSStringFromClass(parentCls);
            LSCExportTypeDescriptor *parentTypeDescriptor = exportTypes[parentName];
            if (!parentTypeDescriptor)
            {
                //创建父级类型
                parentTypeDescriptor = [self _createTypeDescriptorWithName:parentName];
            }
            
            if (!parentTypeDescriptor)
            {
                //证明该类型已经是导出类型的根类型，则其父级类型为Object
                parentTypeDescriptor = [LSCExportTypeDescriptor objectTypeDescriptor];
            }
            
            //关联父级类型
            typeDescriptor.parentTypeDescriptor = parentTypeDescriptor;
            
            return typeDescriptor;
        }
    }
    
    return nil;
}

/**
 映射类型

 @param name 类型名称
 @param alias 别名
 @return YES 表示映射成功，NO 表示映射失败
 */
+ (BOOL)_mappingTypeWithName:(NSString *)name alias:(NSString *)alias
{
    if (![alias isEqualToString:@"Object"])
    {
        __block NSString *typeName = name;
        __block Class cls = NSClassFromString(typeName);
        if (cls == NULL)
        {
            //由于Swift的类型名称为“模块名称.类型名称”，因此尝试拼接模块名称后进行类型检测
            NSString *fullName = [NSString stringWithFormat:@"%@.%@", [NSProcessInfo processInfo].processName, name];
            cls = NSClassFromString(fullName);
            
            NSMutableArray<NSBundle *> *allBundles = [NSMutableArray array];
            if ([NSBundle allBundles].count > 0)
            {
                [allBundles addObjectsFromArray:[NSBundle allBundles]];
            }
            if ([NSBundle allFrameworks].count > 0)
            {
                [allBundles addObjectsFromArray:[NSBundle allFrameworks]];
            }
            
            [allBundles enumerateObjectsUsingBlock:^(NSBundle * _Nonnull bundle, NSUInteger idx, BOOL * _Nonnull stop) {
                
                NSString *moduleName = bundle.executablePath.lastPathComponent;
                moduleName = [moduleName stringByReplacingOccurrencesOfString:@"-" withString:@"_"];
                
                if (moduleName)
                {
                    NSString *targetName = [NSString stringWithFormat:@"%@.%@", moduleName, name];
                    cls = NSClassFromString(targetName);
                    if (cls != NULL)
                    {
                        typeName = targetName;
                        *stop = YES;
                    }
                }
                
            }];
        }
        
        if (cls != NULL)
        {
            if (class_getClassMethod(cls, @selector(conformsToProtocol:))
                && [cls conformsToProtocol:@protocol(LSCExportType)])
            {
                //记录映射关系
                [exportTypesMapping setObject:typeName forKey:alias];
                return YES;
            }
        }
    }
    
    return NO;
}

/**
 判断指定类型是否有定义指定类方法
 
 @param selector 方法名称
 @param class 类型
 @return YES 表示有实现， NO 表示没有
 */
+ (BOOL)_declareClassMethodResponderToSelector:(SEL)selector withClass:(Class)class
{
    Class metaCls = objc_getMetaClass(NSStringFromClass(class).UTF8String);
    
    uint count = 0;
    Method *methodList = class_copyMethodList(metaCls, &count);
    for (int i = 0; i < count; i++)
    {
        if (method_getName(*(methodList + i)) == selector)
        {
            return YES;
        }
    }
    
    free(methodList);
    
    return NO;
}

#pragma mark - Private

/**
 设置导出环境
 */
- (void)_setupExportEnv
{
    [self.context.optQueue performAction:^{
        
        //为_G设置元表，用于监听其对象的获取，从而找出哪些是导出类型
        lua_State *state = self.context.currentSession.state;
        [LSCEngineAdapter getGlobal:state name:"_G"];
        
        if (![LSCEngineAdapter isTable:state index:-1])
        {
            [self.context.currentSession reportLuaExceptionWithMessage:@"Invalid '_G' object，setup the exporter fail."];
            [LSCEngineAdapter pop:state count:1];
            return;
        }
        
        //创建_G元表
        [LSCEngineAdapter newTable:state];
        
        //监听__index元方法
        [LSCEngineAdapter pushLightUserdata:(__bridge void *)self state:state];
        [LSCEngineAdapter pushCClosure:globalIndexMetaMethodHandler n:1 state:state];
        [LSCEngineAdapter setField:state index:-2 name:"__index"];
        
        //绑定为_G元表
        [LSCEngineAdapter setMetatable:state index:-2];
        
        [LSCEngineAdapter pop:state count:1];
        
    }];
}

/**
 准备导出类型到Lua中

 @param typeDescriptor 类型描述
 */
- (void)_prepareExportsTypeWithDescriptor:(LSCExportTypeDescriptor *)typeDescriptor
{
    lua_State *state = self.context.currentSession.state;
    
    //判断父类是否为导出类型
    if (typeDescriptor.parentTypeDescriptor)
    {
        [self.context.optQueue performAction:^{
            //导入父级类型
            [LSCEngineAdapter getGlobal:state name:typeDescriptor.parentTypeDescriptor.typeName.UTF8String];
            [LSCEngineAdapter pop:state count:1];
        }];
    }
    
    [self _exportsType:typeDescriptor state:state];
}

/**
 导出类型

 @param typeDescriptor 类型描述
 @param state Lua状态
 */
- (void)_exportsType:(LSCExportTypeDescriptor *)typeDescriptor state:(lua_State *)state
{
    [self.context.optQueue performAction:^{
        
        //创建类模块
        [LSCEngineAdapter newTable:state];
        
        //设置类名, since ver 1.3
        [LSCEngineAdapter pushString:typeDescriptor.typeName.UTF8String state:state];
        [LSCEngineAdapter setField:state index:-2 name:"name"];
        
        //构造函数
        [LSCEngineAdapter pushLightUserdata:(__bridge void *)self state:state];
        [LSCEngineAdapter pushCClosure:objectCreateHandler n:1 state:state];
        [LSCEngineAdapter setField:state index:-2 name:"__call"];
        
        //关联本地类型
        [LSCEngineAdapter pushLightUserdata:(__bridge void *)(typeDescriptor) state:state];
        [LSCEngineAdapter setField:state index:-2 name:"_nativeType"];
        
        /**
         fixed : 由于OC中类方法存在继承关系，因此，直接导出某个类定义的类方法无法满足这种继承关系。
         例如：moduleName方法在Object中定义，但是当其子类调用时由于只能取到当前导出方法的类型(Object)，无法取到调用方法的类型(即Object的子类)，因此导致逻辑处理的异常。
         所以，该处改为导出其继承的所有类方法来满足该功能需要。
         **/
        //导出声明的类方法
        [self _exportsClassMethods:typeDescriptor state:state];
        
        //关联索引
        [LSCEngineAdapter pushValue:-1 state:state];
        [LSCEngineAdapter setField:state index:-2 name:"__index"];
        
        //类型描述
        [LSCEngineAdapter pushLightUserdata:(__bridge void *)self state:state];
        [LSCEngineAdapter pushLightUserdata:(__bridge void *)typeDescriptor state:state];
        [LSCEngineAdapter pushCClosure:classToStringHandler n:2 state:state];
        [LSCEngineAdapter setField:state index:-2 name:"__tostring"];
        
        //获取父类型
        LSCExportTypeDescriptor *parentTypeDescriptor = typeDescriptor.parentTypeDescriptor;
        
        //关联父类模块
        if (parentTypeDescriptor)
        {
            //存在父类，则直接设置父类为元表
            [LSCEngineAdapter getGlobal:state name:parentTypeDescriptor.typeName.UTF8String];
            if ([LSCEngineAdapter isTable:state index:-1])
            {
                //设置父类指向
                [LSCEngineAdapter pushValue:-1 state:state];
                [LSCEngineAdapter setField:state index:-3 name:"super"];
                
                //关联元表
                [LSCEngineAdapter setMetatable:state index:-2];
            }
            else
            {
                [LSCEngineAdapter pop:state count:1];
            }
        }
        else
        {
            //添加子类化对象方法
            [LSCEngineAdapter pushLightUserdata:(__bridge void *)self state:state];
            [LSCEngineAdapter pushCClosure:subClassHandler n:1 state:state];
            [LSCEngineAdapter setField:state index:-2 name:"subclass"];
            
            //增加子类判断方法, since ver 1.3
            [LSCEngineAdapter pushLightUserdata:(__bridge void *)self state:state];
            [LSCEngineAdapter pushCClosure:subclassOfHandler n:1 state:state];
            [LSCEngineAdapter setField:state index:-2 name:"subclassOf"];
            
            //Object需要创建一个typeMapping方法，用于绑定原生类型，其方法原型: function Object.typeMapping(platform, nativeTypeName, alias);
            //该方法在默认情况下无法找到原生类型时使用（默认情况会根据lua调用的类型名称类查询原生类型，如果名称不匹配则表示不存在），允许使用该方法来关联原生类型
            //其中platform为平台字符串类型，分别为：ios,android,u3d
            //nativeTypeName表示要关联的原生类型名称，为类型全称（包含名称空间+类型名称）
            //alias表示类型在lua中表示的名称，可选，如果不填则取原生类型名称，否则使用传入名称
            [LSCEngineAdapter pushLightUserdata:(__bridge void *)self state:state];
            [LSCEngineAdapter pushCClosure:typeMappingHandler n:1 state:state];
            [LSCEngineAdapter setField:state index:-2 name:"typeMapping"];
            
            //Object需要创建一个新table来作为元表，否则无法使用元方法，如：print(Object);
            [LSCEngineAdapter newTable:state];
            
            //构造函数, Object需要在其元表中添加该构造方法
            [LSCEngineAdapter pushLightUserdata:(__bridge void *)self state:state];
            [LSCEngineAdapter pushCClosure:objectCreateHandler n:1 state:state];
            [LSCEngineAdapter setField:state index:-2 name:"__call"];
            
            //类型描述
            [LSCEngineAdapter pushLightUserdata:(__bridge void *)self state:state];
            [LSCEngineAdapter pushCClosure:classToStringHandler n:1 state:state];
            [LSCEngineAdapter setField:state index:-2 name:"__tostring"];
            
            [LSCEngineAdapter setMetatable:state index:-2];
        }
        
        [LSCEngineAdapter setGlobal:state name:typeDescriptor.typeName.UTF8String];
        
        //---------创建实例对象原型表---------------
        [LSCEngineAdapter newMetatable:state name:typeDescriptor.prototypeTypeName.UTF8String];
        
        [LSCEngineAdapter getGlobal:state name:typeDescriptor.typeName.UTF8String];
        [LSCEngineAdapter setField:state index:-2 name:"class"];
        
        [LSCEngineAdapter pushLightUserdata:(__bridge void *)(typeDescriptor) state:state];
        [LSCEngineAdapter setField:state index:-2 name:"_nativeType"];
        
        [LSCEngineAdapter pushValue:-1 state:state];
        [LSCEngineAdapter setField:state index:-2 name:"__index"];
        
        //增加__newindex元方法监听，主要用于原型中注册属性
        [LSCEngineAdapter pushLightUserdata:(__bridge void *)self state:state];
        [LSCEngineAdapter pushCClosure:prototypeNewIndexHandler n:1 state:state];
        [LSCEngineAdapter setField:state index:-2 name:"__newindex"];
        
        [LSCEngineAdapter pushLightUserdata:(__bridge void *)self state:state];
        [LSCEngineAdapter pushLightUserdata:(__bridge void *)typeDescriptor state:state];
        [LSCEngineAdapter pushCClosure:prototypeToStringHandler n:2 state:state];
        [LSCEngineAdapter setField:state index:-2 name:"__tostring"];
        
        //给类元表绑定该实例元表
        [LSCEngineAdapter getGlobal:state name:typeDescriptor.typeName.UTF8String];
        [LSCEngineAdapter pushValue:-2 state:state];
        [LSCEngineAdapter setField:state index:-2 name:"prototype"];
        [LSCEngineAdapter pop:state count:1];
        
        //导出属性
        NSArray<NSString *> *propertySelectorList = [self _exportsProperties:typeDescriptor state:state];
        
        //导出实例方法
        [self _exportsInstanceMethods:typeDescriptor
                 propertySelectorList:propertySelectorList
                                state:state];
        
        if (parentTypeDescriptor)
        {
            //关联父类
            [LSCEngineAdapter getMetatable:state name:parentTypeDescriptor.prototypeTypeName.UTF8String];
            if ([LSCEngineAdapter isTable:state index:-1])
            {
                //设置父类访问属性 since ver 1.3
                [LSCEngineAdapter pushValue:-1 state:state];
                [LSCEngineAdapter setField:state index:-3 name:"super"];
                
                //设置父类元表
                [LSCEngineAdapter setMetatable:state index:-2];
            }
            else
            {
                [LSCEngineAdapter pop:state count:1];
            }
        }
        else
        {
            //Object需要创建一个新table来作为元表，否则无法使用元方法，如：print(Object);
            [LSCEngineAdapter newTable:state];
            
            //增加__newindex元方法监听，主要用于Object原型中注册属性
            [LSCEngineAdapter pushLightUserdata:(__bridge void *)self state:state];
            [LSCEngineAdapter pushCClosure:prototypeNewIndexHandler n:1 state:state];
            [LSCEngineAdapter setField:state index:-2 name:"__newindex"];
            
            [LSCEngineAdapter pushLightUserdata:(__bridge void *)self state:state];
            [LSCEngineAdapter pushCClosure:prototypeToStringHandler n:1 state:state];
            [LSCEngineAdapter setField:state index:-2 name:"__tostring"];
            
            [LSCEngineAdapter setMetatable:state index:-2];
            
            //Object类需要增加一些特殊方法
            //创建instanceOf方法 since ver 1.3
            [LSCEngineAdapter pushLightUserdata:(__bridge void *)self state:state];
            [LSCEngineAdapter pushLightUserdata:(__bridge void *)typeDescriptor state:state];
            [LSCEngineAdapter pushCClosure:instanceOfHandler n:2 state:state];
            [LSCEngineAdapter setField:state index:-2 name:"instanceOf"];
        }
        
        [LSCEngineAdapter pop:state count:1];
        
    }];
}

- (void)_exportsClassMethods:(LSCExportTypeDescriptor *)typeDescriptor
                  targetType:(LSCExportTypeDescriptor *)targetTypeDescriptor
                       state:(lua_State *)state
{
    if (targetTypeDescriptor.nativeType != NULL)
    {
        NSArray *excludesMethodNames = nil;
        Class metaType = objc_getMetaClass(NSStringFromClass(targetTypeDescriptor.nativeType).UTF8String);
        
        //先判断是否有实现注解的排除类方法
        if (class_conformsToProtocol(targetTypeDescriptor.nativeType, @protocol(LSCExportTypeAnnotation)))
        {
            if ([LSCExportsTypeManager _declareClassMethodResponderToSelector:@selector(excludeExportClassMethods) withClass:targetTypeDescriptor.nativeType])
            {
                excludesMethodNames = [targetTypeDescriptor.nativeType excludeExportClassMethods];
            }
        }
        
        NSArray *builtInExcludeMethodNames = @[NSStringFromSelector(@selector(excludeExportClassMethods)),
                                               NSStringFromSelector(@selector(excludeProperties)),
                                               NSStringFromSelector(@selector(excludeExportInstanceMethods))];
        
        //解析方法
        NSMutableDictionary *methodDict = [typeDescriptor.classMethods mutableCopy];
        if (!methodDict)
        {
            methodDict = [NSMutableDictionary dictionary];
        }
        
        [self.context.optQueue performAction:^{
            
            unsigned int methodCount = 0;
            Method *methods = class_copyMethodList(metaType, &methodCount);
            for (const Method *m = methods; m < methods + methodCount; m ++)
            {
                SEL selector = method_getName(*m);
                
                NSString *selectorName = NSStringFromSelector(selector);
                if (![selectorName hasPrefix:@"_"]
                    && ![selectorName hasPrefix:@"."]
                    && ![builtInExcludeMethodNames containsObject:selectorName]
                    && ![excludesMethodNames containsObject:selectorName])
                {
                    NSString *luaMethodName = [self _getLuaMethodNameWithSelectorName:selectorName];
                    
                    //判断是否已导出
                    __block BOOL hasExists = NO;
                    [LSCEngineAdapter getField:state index:-1 name:luaMethodName.UTF8String];
                    if (![LSCEngineAdapter isNil:state index:-1])
                    {
                        hasExists = YES;
                    }
                    [LSCEngineAdapter pop:state count:1];
                    
                    if (!hasExists)
                    {
                        [LSCEngineAdapter pushLightUserdata:(__bridge void *)self state:state];
                        [LSCEngineAdapter pushString:luaMethodName.UTF8String state:state];
                        [LSCEngineAdapter pushCClosure:classMethodRouteHandler n:2 state:state];
                        
                        [LSCEngineAdapter setField:state index:-2 name:luaMethodName.UTF8String];
                    }
                    
                    NSMutableArray<LSCExportMethodDescriptor *> *methodList = methodDict[luaMethodName];
                    if (!methodList)
                    {
                        methodList = [NSMutableArray array];
                        [methodDict setObject:methodList forKey:luaMethodName];
                    }
                    
                    //获取方法签名
                    NSString *signStr = [self _getMethodSign:*m];
                    
                    hasExists = NO;
                    [methodList enumerateObjectsUsingBlock:^(LSCExportMethodDescriptor * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
                        
                        if ([obj.paramsSignature isEqualToString:signStr])
                        {
                            hasExists = YES;
                            *stop = YES;
                        }
                        
                    }];
                    
                    if (!hasExists)
                    {
                        NSMethodSignature *sign = [targetTypeDescriptor.nativeType methodSignatureForSelector:selector];
                        LSCExportMethodDescriptor *methodDesc = [[LSCExportMethodDescriptor alloc] initWithSelector:selector methodSignature:sign paramsSignature:signStr];
                        
                        [methodList addObject:methodDesc];
                    }
                    
                }
            }
            free(methods);
            
        }];
        
        
        typeDescriptor.classMethods = methodDict;
    }
    
    //导出父级方法
    LSCExportTypeDescriptor *parentTypeDescriptor = targetTypeDescriptor.parentTypeDescriptor;
    if (parentTypeDescriptor)
    {
        [self _exportsClassMethods:typeDescriptor
                        targetType:parentTypeDescriptor
                             state:state];
    }
}


/**
 导出类方法

 @param typeDescriptor 类型
 @param state Lua状态
 */
- (void)_exportsClassMethods:(LSCExportTypeDescriptor *)typeDescriptor
                       state:(lua_State *)state
{
    [self _exportsClassMethods:typeDescriptor
                    targetType:typeDescriptor
                         state:state];
}


/**
 导出属性

 @param typeDescriptor 类型
 @param state 状态
 
 @return 属性Selector名称集合，用于在导出方法时过滤属性的Getter和Setter
 */
- (NSArray<NSString *> *)_exportsProperties:(LSCExportTypeDescriptor *)typeDescriptor
                                      state:(lua_State *)state
{
    NSMutableSet<NSString *> *propertySelectorList = nil;
    if (typeDescriptor.nativeType != NULL)
    {
        propertySelectorList = [NSMutableSet set];
        
        NSMutableDictionary *propertiesDict = [NSMutableDictionary dictionary];
        
        //注册属性
        //先判断是否有注解排除实例方法
        NSArray *excludesPropertyNames = nil;
        if (class_conformsToProtocol(typeDescriptor.nativeType, @protocol(LSCExportTypeAnnotation)))
        {
            if ([LSCExportsTypeManager _declareClassMethodResponderToSelector:@selector(excludeProperties) withClass:typeDescriptor.nativeType])
            {
                excludesPropertyNames = [typeDescriptor.nativeType excludeProperties];
            }
        }
        
        uint count = 0;
        objc_property_t *properties = class_copyPropertyList(typeDescriptor.nativeType, &count);

        for (int i = 0; i < count; i++)
        {
            objc_property_t property = *(properties + i);
            NSString *propertyName = [NSString stringWithUTF8String:property_getName(property)];
            
            //获取属性特性
            BOOL readonly = NO;
            NSString *getterName = nil;
            NSString *setterName = nil;
            uint attrCount = 0;
            objc_property_attribute_t *attrs = property_copyAttributeList(property, &attrCount);
            for (int j = 0; j < attrCount; j++)
            {
                objc_property_attribute_t attr = *(attrs + j);
                if (strcmp(attr.name, "G") == 0)
                {
                    getterName = [NSString stringWithUTF8String:attr.value];
                }
                else if (strcmp(attr.name, "S") == 0)
                {
                    //Setter
                    setterName = [NSString stringWithUTF8String:attr.value];
                }
                else if (strcmp(attr.name, "R") == 0)
                {
                    //只读属性
                    readonly = YES;
                }
            }
            free(attrs);
            
            if (!getterName)
            {
                getterName = propertyName;
            }
            if (!setterName)
            {
                setterName = [NSString stringWithFormat:@"set%@%@:",
                              [propertyName.capitalizedString substringToIndex:1],
                              [propertyName substringFromIndex:1]];
            }
            
            if (!readonly)
            {
                [propertySelectorList addObject:setterName];
            }
            [propertySelectorList addObject:getterName];
            
            if ([propertyName hasPrefix:@"_"]
                || [propertyName isEqualToString:@"hash"]
                || [propertyName isEqualToString:@"superclass"]
                || [propertyName isEqualToString:@"description"]
                || [propertyName isEqualToString:@"debugDescription"]
                || [excludesPropertyNames containsObject:propertyName])
            {
                continue;
            }
            
            //生成导出属性
            LSCExportPropertyDescriptor *propertyDescriptor = [[LSCExportPropertyDescriptor alloc] initWithName:propertyName getterSelector:NSSelectorFromString(getterName) setterSelector:readonly ? nil : NSSelectorFromString(setterName)];
            
            [propertiesDict setObject:propertyDescriptor forKey:propertyDescriptor.name];
        }
        
        free(properties);
        
        //记录属性Selector名称集合
        typeDescriptor.propertySelectorNames = propertySelectorList;
        //记录导出属性
        typeDescriptor.properties = propertiesDict;
        
        //加入所有父类属性名称集合，用于检测当前类型是否有重载父级属性
        LSCExportTypeDescriptor *parentTypeDescriptor = typeDescriptor.parentTypeDescriptor;
        while (parentTypeDescriptor)
        {
            [propertySelectorList addObjectsFromArray:parentTypeDescriptor.propertySelectorNames.allObjects];
            parentTypeDescriptor = parentTypeDescriptor.parentTypeDescriptor;
        }
    }
    
    return [propertySelectorList allObjects];
}

/**
 导出实例方法

 @param typeDescriptor 类型
 @param propertySelectorList 属性Getter／Setter列表
 @param state Lua状态
 */
- (void)_exportsInstanceMethods:(LSCExportTypeDescriptor *)typeDescriptor
           propertySelectorList:(NSArray<NSString *> *)propertySelectorList
                          state:(lua_State *)state
{
    if (typeDescriptor.nativeType != NULL)
    {
        //注册实例方法
        //先判断是否有注解排除实例方法
        NSArray *excludesMethodNames = nil;
        if (class_conformsToProtocol(typeDescriptor.nativeType, @protocol(LSCExportTypeAnnotation)))
        {
            if ([LSCExportsTypeManager _declareClassMethodResponderToSelector:@selector(excludeExportInstanceMethods) withClass:typeDescriptor.nativeType])
            {
                excludesMethodNames = [typeDescriptor.nativeType excludeExportInstanceMethods];
            }
        }
        
        NSArray *buildInExcludeMethodNames = @[];
        
        //解析方法
        NSMutableDictionary *methodDict = [typeDescriptor.instanceMethods mutableCopy];
        if (!methodDict)
        {
            methodDict = [NSMutableDictionary dictionary];
        }
        
        [self.context.optQueue performAction:^{
            
            unsigned int methodCount = 0;
            Method *methods = class_copyMethodList(typeDescriptor.nativeType, &methodCount);
            for (const Method *m = methods; m < methods + methodCount; m ++)
            {
                SEL selector = method_getName(*m);
                
                NSString *methodName = NSStringFromSelector(selector);
                
                if (![methodName hasPrefix:@"_"]
                         && ![methodName hasPrefix:@"."]
                         && ![methodName isEqualToString:@"dealloc"]
                         && ![buildInExcludeMethodNames containsObject:methodName]
                         && ![propertySelectorList containsObject:methodName]
                         && ![excludesMethodNames containsObject:methodName])
                {
                    NSString *luaMethodName = [self _getLuaMethodNameWithSelectorName:methodName];
                    
                    if (![luaMethodName isEqualToString:@"init"])
                    {
                        //非初始化方法需要进行导出
                        //判断是否已导出
                        __block BOOL hasExists = NO;
                        [LSCEngineAdapter getField:state index:-1 name:luaMethodName.UTF8String];
                        if (![LSCEngineAdapter isNil:state index:-1])
                        {
                            hasExists = YES;
                        }
                        [LSCEngineAdapter pop:state count:1];
                        
                        if (!hasExists)
                        {
                            [LSCEngineAdapter pushLightUserdata:(__bridge void *)self state:state];
                            [LSCEngineAdapter pushLightUserdata:(__bridge void *)typeDescriptor state:state];
                            [LSCEngineAdapter pushString:luaMethodName.UTF8String state:state];
                            [LSCEngineAdapter pushCClosure:instanceMethodRouteHandler n:3 state:state];
                            
                            [LSCEngineAdapter setField:state index:-2 name:luaMethodName.UTF8String];
                        }
                    }
                    
                    NSMutableArray<LSCExportMethodDescriptor *> *methodList = methodDict[luaMethodName];
                    if (!methodList)
                    {
                        methodList = [NSMutableArray array];
                        [methodDict setObject:methodList forKey:luaMethodName];
                    }
                    
                    //获取方法签名
                    NSString *signStr = [self _getMethodSign:*m];
                    
                    __block BOOL hasExists = NO;
                    [methodList enumerateObjectsUsingBlock:^(LSCExportMethodDescriptor * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
                        
                        if ([obj.paramsSignature isEqualToString:signStr])
                        {
                            hasExists = YES;
                            *stop = YES;
                        }
                        
                    }];
                    
                    if (!hasExists)
                    {
                        NSMethodSignature *sign = [typeDescriptor.nativeType instanceMethodSignatureForSelector:selector];
                        LSCExportMethodDescriptor *methodDesc = [[LSCExportMethodDescriptor alloc] initWithSelector:selector methodSignature:sign paramsSignature:signStr];
                        
                        [methodList addObject:methodDesc];
                    }
                }
            }
            free(methods);
            
        }];
        
        typeDescriptor.instanceMethods = methodDict;
    }
}

/**
 根据Selector名称获取Lua中的方法名称

 @param selectorName Selector名称
 @return Lua中的方法名
 */
- (NSString *)_getLuaMethodNameWithSelectorName:(NSString *)selectorName
{
    NSString *luaName = selectorName;

    if ([luaName hasPrefix:@"init"])
    {
        //检测是否为初始化方法
        if (luaName.length > 4)
        {
            unichar ch = [luaName characterAtIndex:4];
            if (ch > 'A' && ch < 'Z')
            {
                return @"init";
            }
        }
        else
        {
            return @"init";
        }
    }
    
    if (self.context.config.fullExportName)
    {
        //使用完整限定名称
        luaName = [selectorName stringByReplacingOccurrencesOfString:@":" withString:@"_"];
        if ([luaName hasSuffix:@"_"])
        {
            luaName = [luaName substringWithRange:NSMakeRange(0, luaName.length - 1)];
        }
    }
    else
    {
        NSRange range = [luaName rangeOfString:@":"];
        if (range.location != NSNotFound)
        {
            luaName = [luaName substringToIndex:range.location];
        }
        
        range = [luaName rangeOfString:@"With"];
        if (range.location != NSNotFound)
        {
            luaName = [luaName substringToIndex:range.location];
        }
        
        range = [luaName rangeOfString:@"At"];
        if (range.location != NSNotFound)
        {
            luaName = [luaName substringToIndex:range.location];
        }
        
        range = [luaName rangeOfString:@"By"];
        if (range.location != NSNotFound)
        {
            luaName = [luaName substringToIndex:range.location];
        }
    }
    

    return luaName;
}

/**
 创建原生对象实例
 
 @param object 类型实例对象
 @param typeDescriptor 类型
 @param state 状态
 @param queue 队列
 */
- (void)_initLuaObjectWithObject:(id)object
                            type:(LSCExportTypeDescriptor *)typeDescriptor
                           state:(lua_State *)state
                           queue:(LSCOperationQueue *)queue;
{
    void (^handler) (void) = ^{
        
        int errFuncIndex = [self.context catchLuaExceptionWithState:state queue:queue];
        
        [self _attachLuaInstanceWithNativeObject:object
                                            type:typeDescriptor
                                           state:state
                                           queue:queue];
        
        //通过_createLuaInstanceWithState方法后会创建实例并放入栈顶
        //调用实例对象的init方法
        [LSCEngineAdapter getField:state index:-1 name:"init"];
        if ([LSCEngineAdapter isFunction:state index:-1])
        {
            [LSCEngineAdapter pushValue:-2 state:state];
            
            //将create传入的参数传递给init方法
            //-4 代表有4个非参数值在栈中，由栈顶开始计算，分别是：实例对象，init方法，实例对象，异常捕获方法
            int paramCount = [LSCEngineAdapter getTop:state] - 4;
            //从索引2开始传入参数，ver2.2后使用冒号调用create方法，忽略第一个参数self
            for (int i = 2; i <= paramCount; i++)
            {
                [LSCEngineAdapter pushValue:i state:state];
            }
            
            [LSCEngineAdapter pCall:state nargs:paramCount nresults:0 errfunc:errFuncIndex];
        }
        else
        {
            [LSCEngineAdapter pop:state count:1];       //出栈init方法
        }
        
        //移除异常捕获方法
        [LSCEngineAdapter remove:state index:errFuncIndex];
        
    };
    
    if (queue)
    {
        [queue performAction:handler];
    }
    else
    {
        handler ();
    }
}

/**
 将一个原生对象附加到Lua对象中
 
 @param nativeObject 原生实例对象
 @param typeDescriptor 类型描述
 */
- (void)_attachLuaInstanceWithNativeObject:(id)nativeObject
                                      type:(LSCExportTypeDescriptor *)typeDescriptor
                                     state:(lua_State *)state
                                     queue:(LSCOperationQueue *)queue
{
    void (^handler)(void) = ^{
        
        //先为实例对象在lua中创建内存
        LSCUserdataRef ref = (LSCUserdataRef)[LSCEngineAdapter newUserdata:state size:sizeof(LSCUserdataRef)];
        if (nativeObject)
        {
            //创建本地实例对象，赋予lua的内存块并进行保留引用
            ref -> value = (void *)CFBridgingRetain(nativeObject);
        }
        
        //创建一个临时table作为元表，用于在lua上动态添加属性或方法
        [LSCEngineAdapter newTable:state];
        
        ///变更索引为function，实现动态路由
        [LSCEngineAdapter pushLightUserdata:(__bridge void *)self state:state];
        [LSCEngineAdapter pushLightUserdata:(__bridge void *)typeDescriptor state:state];
        [LSCEngineAdapter pushLightUserdata:(__bridge void *)nativeObject state:state];
        [LSCEngineAdapter pushCClosure:instanceIndexHandler n:3 state:state];
        [LSCEngineAdapter setField:state index:-2 name:"__index"];
        
        [LSCEngineAdapter pushLightUserdata:(__bridge void *)self state:state];
        [LSCEngineAdapter pushLightUserdata:(__bridge void *)typeDescriptor state:state];
        [LSCEngineAdapter pushLightUserdata:(__bridge void *)nativeObject state:state];
        [LSCEngineAdapter pushCClosure:instanceNewIndexHandler n:3 state:state];
        [LSCEngineAdapter setField:state index:-2 name:"__newindex"];
        
        [LSCEngineAdapter pushLightUserdata:(__bridge void *)self state:state];
        [LSCEngineAdapter pushCClosure:objectDestroyHandler n:1 state:state];
        [LSCEngineAdapter setField:state index:-2 name:"__gc"];
        
        [LSCEngineAdapter pushLightUserdata:(__bridge void *)self state:state];
        [LSCEngineAdapter pushCClosure:objectToStringHandler n:1 state:state];
        [LSCEngineAdapter setField:state index:-2 name:"__tostring"];
        
        [LSCEngineAdapter pushValue:-1 state:state];
        [LSCEngineAdapter setMetatable:state index:-3];
        
        [LSCEngineAdapter getMetatable:state name:typeDescriptor.prototypeTypeName.UTF8String];
        if ([LSCEngineAdapter isTable:state index:-1])
        {
            [LSCEngineAdapter setMetatable:state index:-2];
        }
        else
        {
            [LSCEngineAdapter pop:state count:1];
        }
        
        [LSCEngineAdapter pop:state count:1];
        
        //将创建对象放入到_vars_表中，主要修复对象创建后，在init中调用方法或者访问属性，由于对象尚未记录在_vars_中，而循环创建lua对象，并导致栈溢出。
        NSString *objectId = [NSString stringWithFormat:@"%p", nativeObject];
        [self.context.dataExchanger setLubObjectByStackIndex:-1 objectId:objectId];
        
    };
    
    if (queue)
    {
        [queue performAction:handler];
    }
    else
    {
        handler();
    }
    
}

/**
 返回对象在Lua中的类型描述

 @param object 对象实例
 @return 类型描述，如果为nil则表示非导出类型
 */
- (LSCExportTypeDescriptor *)_typeDescriptorWithObject:(id)object
{
    if ([object conformsToProtocol:@protocol(LSCExportType)])
    {
        NSString *clsName = NSStringFromClass([object class]);
        return [LSCExportsTypeManager _getMappingTypeWithName:clsName];
    }
    else if ([object isKindOfClass:[LSCVirtualInstance class]])
    {
        //为Lua层类型
        return ((LSCVirtualInstance *)object).typeDescriptor;
    }
    
    return nil;
}

/**
 获取调用器

 @param methodName 方法名
 @param arguments 参数列表
 @param typeDesc 类型
 @param isStatic 是否为类方法
 @return 调用器对象
 */
- (NSInvocation *)_invocationWithMethodName:(NSString *)methodName
                                  arguments:(NSArray *)arguments
                                   typeDesc:(LSCExportTypeDescriptor *)typeDesc
                                   isStatic:(BOOL)isStatic
{
    LSCExportMethodDescriptor *methodDesc = nil;
    if (isStatic)
    {
        methodDesc = [typeDesc classMethodWithName:methodName arguments:arguments];
    }
    else
    {
        methodDesc = [typeDesc instanceMethodWithName:methodName arguments:arguments];
    }
    
    return [methodDesc createInvocation];
}


/**
 获取方法签名

 @param method 方法
 @return 签名字符串
 */
- (NSString *)_getMethodSign:(Method)method
{
    NSMutableString *signStr = [NSMutableString string];
    int argCount = method_getNumberOfArguments(method);
    for (int i = 2; i < argCount; i++)
    {
        char s[256] = {0};
        method_getArgumentType(method, i, s, 256);
        [signStr appendString:[NSString stringWithUTF8String:s]];
    }
    
    return signStr;
}


/**
 查找实例导出属性描述

 @param session 会话
 @param typeDescriptor 类型描述
 @param propertyName 属性名称
 @return 属性描述对象
 */
- (LSCExportPropertyDescriptor *)_findInstancePropertyWithSession:(LSCSession *)session
                                                   typeDescriptor:(LSCExportTypeDescriptor *)typeDescriptor
                                                     propertyName:(NSString *)propertyName
{
    __block LSCExportPropertyDescriptor *propertyDescriptor = nil;
    
    [self.context.optQueue performAction:^{

        lua_State *state = session.state;
        if (typeDescriptor)
        {
            [LSCEngineAdapter getMetatable:state name:typeDescriptor.prototypeTypeName.UTF8String];
            [LSCEngineAdapter pushString:propertyName.UTF8String state:state];
            [LSCEngineAdapter rawGet:state index:-2];
            
            if ([LSCEngineAdapter isNil:state index:-1])
            {
                //不存在
                propertyDescriptor = [typeDescriptor.properties objectForKey:propertyName];
                if (!propertyDescriptor)
                {
                    if (typeDescriptor.parentTypeDescriptor)
                    {
                        //递归父类
                        propertyDescriptor = [self _findInstancePropertyWithSession:session
                                                                     typeDescriptor:typeDescriptor.parentTypeDescriptor
                                                                       propertyName:propertyName];
                    }
                }
            }
            
            [LSCEngineAdapter pop:state count:2];
        }
        
    }];
    
    
    return propertyDescriptor;
}

/**
 获取实例属性描述

 @param session 会话
 @param instance 实例对象
 @param typeDescriptor 类型描述
 @param propertyName 属性名称
 
 @return 返回值数量
 */
- (int)_instancePropertyWithSession:(LSCSession *)session
                           instance:(id)instance
                     typeDescriptor:(LSCExportTypeDescriptor *)typeDescriptor
                       propertyName:(NSString *)propertyName
{
    __block int retValueCount = 1;
    
    [self.context.optQueue performAction:^{
        
        lua_State *state = session.state;
        if (typeDescriptor)
        {
            [LSCEngineAdapter getMetatable:state name:typeDescriptor.prototypeTypeName.UTF8String];
            [LSCEngineAdapter pushString:propertyName.UTF8String state:state];
            [LSCEngineAdapter rawGet:state index:-2];
            
            if ([LSCEngineAdapter isNil:state index:-1])
            {
                [LSCEngineAdapter pop:state count:1];
                
                //不存在
                LSCExportPropertyDescriptor *propertyDescriptor = [typeDescriptor.properties objectForKey:propertyName];
                if (!propertyDescriptor)
                {
                    if (typeDescriptor.parentTypeDescriptor)
                    {
                        //递归父类
                        retValueCount = [self _instancePropertyWithSession:session
                                                                  instance:instance
                                                            typeDescriptor:typeDescriptor.parentTypeDescriptor
                                                              propertyName:propertyName];
                    }
                    else
                    {
                        [LSCEngineAdapter pushNil:state];
                    }
                }
                else
                {
                    @try
                    {
                        LSCValue *retValue = [propertyDescriptor invokeGetterWithInstance:instance
                                                                           typeDescriptor:typeDescriptor];
                        retValueCount = [session setReturnValue:retValue];
                    }
                    @catch (NSException *exception)
                    {
                        NSString *errMsg = [NSString stringWithFormat:@"get property fail: %@", exception.reason];
                        [session reportLuaExceptionWithMessage:errMsg];
                    }
                    
                }
            }
            
            [LSCEngineAdapter remove:state index:-1-retValueCount];
        }
        
    }];
    
    
    return retValueCount;
}

#pragma mark - C Method


/**
 类型映射

 @param state 状态
 @return 返回参数数量
 */
static int typeMappingHandler(lua_State *state)
{
    int index = [LSCEngineAdapter upvalueIndex:1];
    const void *ptr = [LSCEngineAdapter toPointer:state index:index];
    LSCExportsTypeManager *exporter = (__bridge LSCExportsTypeManager *)ptr;
    
    LSCSession *callSession = [exporter.context makeSessionWithState:state lightweight:NO];
    
    if ([LSCEngineAdapter type:state index:1] != LUA_TTABLE)
    {
        [callSession reportLuaExceptionWithMessage:@"please use the colon syntax to call the method"];
    }
    else if ([LSCEngineAdapter getTop:state] < 4)
    {
        NSString *errMsg = @"`typeMapping` method need to pass 3 parameters";
        [callSession reportLuaExceptionWithMessage:errMsg];
    }
    else
    {
        NSString *platform = [NSString stringWithUTF8String:[LSCEngineAdapter toString:state index:2]];
        if ([[platform lowercaseString] isEqualToString:@"ios"])
        {
            NSString *nativeTypeName = [NSString stringWithUTF8String:[LSCEngineAdapter toString:state index:3]];
            NSString *alias = [NSString stringWithUTF8String:[LSCEngineAdapter toString:state index:4]];
            [LSCExportsTypeManager _mappingTypeWithName:nativeTypeName alias:alias];
        }
    }
    
    [exporter.context destroySession:callSession];
    
    return 0;
}

/**
 类方法路由处理器

 @param state 状态
 @return 返回参数数量
 */
static int classMethodRouteHandler(lua_State *state)
{
    int retCount = 0;
    
    int index = [LSCEngineAdapter upvalueIndex:1];
    const void *ptr = [LSCEngineAdapter toPointer:state index:index];
    LSCExportsTypeManager *exporter = (__bridge LSCExportsTypeManager *)ptr;
    
    index = [LSCEngineAdapter upvalueIndex:2];
    const char *methodNameCStr = [LSCEngineAdapter toString:state index:index];
    NSString *methodName = [NSString stringWithUTF8String:methodNameCStr];
    
    LSCSession *callSession = [exporter.context makeSessionWithState:state lightweight:NO];
    
    if ([LSCEngineAdapter type:state index:1] != LUA_TTABLE)
    {
        [callSession reportLuaExceptionWithMessage:@"please use the colon syntax to call the method"];
    }
    else
    {
        LSCExportTypeDescriptor *typeDescriptor = nil;
        [LSCEngineAdapter getField:state index:1 name:"_nativeType"];
        if ([LSCEngineAdapter type:state index:-1] == LUA_TLIGHTUSERDATA)
        {
            typeDescriptor = (__bridge LSCExportTypeDescriptor *)[LSCEngineAdapter toPointer:state index:-1];
        }
        [LSCEngineAdapter pop:state count:1];
        
        if (typeDescriptor)
        {
            NSArray *arguments = [callSession parseArgumentsWithoutTheFirst];
            
            //筛选方法，对于重载方法需要根据lua传入参数进行筛选
            NSInvocation *invocation = [exporter _invocationWithMethodName:methodName
                                                                 arguments:arguments
                                                                  typeDesc:typeDescriptor
                                                                  isStatic:YES];
            
            //确定调用方法的Target
            if (invocation)
            {
                LSCValue *retValue = nil;
                @try
                {
                    retValue = [typeDescriptor _invokeMethodWithInstance:nil
                                                              invocation:invocation
                                                               arguments:arguments];
                }
                @catch (NSException *exception)
                {
                    NSString *errMsg = [NSString stringWithFormat:@"call `%@` method fail : %@", methodName, exception.reason];
                    [callSession reportLuaExceptionWithMessage:errMsg];
                }
                
                if (retValue)
                {
                    retCount = [callSession setReturnValue:retValue];
                }
            }
            else
            {
                NSString *errMsg = [NSString stringWithFormat:@"call `%@` method fail : argument type mismatch", methodName];
                [callSession reportLuaExceptionWithMessage:errMsg];
            }
        }
        else
        {
            NSString *errMsg = [NSString stringWithFormat:@"call `%@` method fail : invalid type", methodName];
            [callSession reportLuaExceptionWithMessage:errMsg];
        }
    }
    
    [exporter.context destroySession:callSession];
    
    return retCount;
}


/**
 实例方法路由处理

 @param state 状态
 @return 参数个数
 */
static int instanceMethodRouteHandler(lua_State *state)
{
    int retCount = 0;

    int index = [LSCEngineAdapter upvalueIndex:1];
    const void *ptr = [LSCEngineAdapter toPointer:state index:index];
    LSCExportsTypeManager *exporter = (__bridge LSCExportsTypeManager *)ptr;

    index = [LSCEngineAdapter upvalueIndex:2];
    ptr = [LSCEngineAdapter toPointer:state index:index];
    LSCExportTypeDescriptor *typeDescriptor = (__bridge LSCExportTypeDescriptor *)ptr;

    index = [LSCEngineAdapter upvalueIndex:3];
    const char *methodNameCStr = [LSCEngineAdapter toString:state index:index];
    NSString *methodName = [NSString stringWithUTF8String:methodNameCStr];
    
    //创建调用会话
    LSCSession *callSession = [exporter.context makeSessionWithState:state lightweight:NO];

    if ([LSCEngineAdapter type:state index:1] != LUA_TUSERDATA)
    {
        NSString *errMsg = [NSString stringWithFormat:@"call %@ method error : missing self parameter, please call by instance:methodName(param)", methodName];
        [callSession reportLuaExceptionWithMessage:errMsg];
    }
    else
    {
        NSArray *arguments = [callSession parseArguments];
        id instance = [arguments[0] toObject];
        
        NSInvocation *invocation = [exporter _invocationWithMethodName:methodName
                                                             arguments:arguments
                                                              typeDesc:typeDescriptor
                                                              isStatic:NO];
        
        //获取类实例对象
        if (invocation && instance)
        {
            LSCValue *retValue = nil;
            
            @try
            {
                retValue = [typeDescriptor _invokeMethodWithInstance:instance
                                                          invocation:invocation
                                                           arguments:arguments];
            }
            @catch (NSException *exception)
            {
                NSString *errMsg = [NSString stringWithFormat:@"call `%@` method fail : %@", methodName, exception.reason];
                [callSession reportLuaExceptionWithMessage:errMsg];
            }
            
            
            if (retValue)
            {
                retCount = [callSession setReturnValue:retValue];
            }
        }
        else
        {
            NSString *errMsg = [NSString stringWithFormat:@"call `%@` method fail : argument type mismatch", methodName];
            [callSession reportLuaExceptionWithMessage:errMsg];
        }
    }

    [exporter.context destroySession:callSession];
    
    return retCount;
}

/**
 *  创建对象时处理
 *
 *  @param state 状态机
 *
 *  @return 参数数量
 */
static int objectCreateHandler (lua_State *state)
{
    int index = [LSCEngineAdapter upvalueIndex:1];
    const void *ptr = [LSCEngineAdapter toPointer:state index:index];
    LSCExportsTypeManager *exporter = (__bridge LSCExportsTypeManager *)ptr;
    
    LSCSession *session = [exporter.context makeSessionWithState:state lightweight:NO];
    
    //获取传入类型
    LSCExportTypeDescriptor *typeDescriptor = nil;
    [LSCEngineAdapter getField:state index:1 name:"_nativeType"];
    if ([LSCEngineAdapter type:state index:-1] == LUA_TLIGHTUSERDATA)
    {
        typeDescriptor = (__bridge LSCExportTypeDescriptor *)[LSCEngineAdapter toPointer:state index:-1];
    }
    [LSCEngineAdapter pop:state count:1];
    
    if (typeDescriptor)
    {
        //创建对象
        id instance = nil;
        if (typeDescriptor.nativeType != NULL)
        {
            //获取参数传入类型，筛选适合的初始化方法
            NSMutableArray *arguments = [[session parseArguments] mutableCopy];
            NSInvocation *invocation = [exporter _invocationWithMethodName:@"init"
                                                                 arguments:arguments
                                                                  typeDesc:typeDescriptor
                                                                  isStatic:NO];
            
            if (invocation)
            {
                @try
                {
                    LSCValue *retValue = [typeDescriptor _invokeMethodWithInstance:[typeDescriptor.nativeType alloc]
                                                                        invocation:invocation
                                                                         arguments:arguments];
                    instance = [retValue toObject];
                }
                @catch (NSException *exception)
                {
                    NSString *errMsg = [NSString stringWithFormat:@"construct instance fail : %@", exception.reason];
                    [session reportLuaExceptionWithMessage:errMsg];
                }
                
            }
            else
            {
                instance = [[typeDescriptor.nativeType alloc] init];
            }
            
        }
        else
        {
            //创建一个虚拟的类型对象
            instance = [[LSCVirtualInstance alloc] initWithTypeDescriptor:typeDescriptor];
        }
        
        [exporter _initLuaObjectWithObject:instance
                                      type:typeDescriptor
                                     state:state
                                     queue:exporter.context.optQueue];
    }
    else
    {
        [session reportLuaExceptionWithMessage:@"construct instance fail : invalid type!"];
    }
    
    [exporter.context destroySession:session];
    
    return 1;
}

/**
 实例对象更新索引处理
 
 @param state 状态机
 @return 参数数量
 */
static int instanceNewIndexHandler (lua_State *state)
{
    LSCExportsTypeManager *exporter = (__bridge LSCExportsTypeManager *)[LSCEngineAdapter toPointer:state
                                                                                  index:[LSCEngineAdapter upvalueIndex:1]];
    LSCExportTypeDescriptor *typeDescriptor = (LSCExportTypeDescriptor *)[LSCEngineAdapter toPointer:state index:[LSCEngineAdapter upvalueIndex:2]];
    id instance = (__bridge id)[LSCEngineAdapter toPointer:state index:[LSCEngineAdapter upvalueIndex:3]];
    NSString *key = [NSString stringWithUTF8String:[LSCEngineAdapter toString:state index:2]];
    
    LSCSession *session = [exporter.context makeSessionWithState:state lightweight:YES];
    
    //检测是否存在类型属性
    LSCExportPropertyDescriptor *propertyDescriptor = [exporter _findInstancePropertyWithSession:session
                                                                                  typeDescriptor:typeDescriptor
                                                                                    propertyName:key];
    if (propertyDescriptor)
    {
        LSCValue *value = [LSCValue tmpValueWithContext:exporter.context atIndex:3];
        
        @try
        {
            [propertyDescriptor invokeSetterWithInstance:instance typeDescriptor:typeDescriptor value:value];
        }
        @catch (NSException *exception)
        {
            NSString *errMsg = [NSString stringWithFormat:@"set `%@` property fail : %@", key, exception.reason];
            [session reportLuaExceptionWithMessage:errMsg];
        }
    }
    else
    {
        //先找到实例对象的元表，向元表添加属性
        [LSCEngineAdapter getMetatable:state index:1];
        if ([LSCEngineAdapter isTable:state index:-1])
        {
            [LSCEngineAdapter pushValue:2 state:state];
            [LSCEngineAdapter pushValue:3 state:state];
            [LSCEngineAdapter rawSet:state index:-3];
        }
        
        [LSCEngineAdapter pop:state count:1];
    }
    
    [exporter.context destroySession:session];
    
    return 0;
}

/**
 实例对象索引方法处理器
 
 @param state 状态
 @return 返回参数数量
 */
static int instanceIndexHandler(lua_State *state)
{
    int retValueCount = 1;
    
    LSCExportsTypeManager *exporter = (LSCExportsTypeManager *)[LSCEngineAdapter toPointer:state index:[LSCEngineAdapter upvalueIndex:1]];
    LSCExportTypeDescriptor *typeDescriptor = (LSCExportTypeDescriptor *)[LSCEngineAdapter toPointer:state index:[LSCEngineAdapter upvalueIndex:2]];
    id instance = (__bridge id)[LSCEngineAdapter toPointer:state index:[LSCEngineAdapter upvalueIndex:3]];
    
    LSCSession *session = [exporter.context makeSessionWithState:state lightweight:YES];
    
    NSString *key = [NSString stringWithUTF8String:[LSCEngineAdapter toString:state index:2]];
    
    //检测元表是否包含指定值
    [LSCEngineAdapter getMetatable:state index:1];
    [LSCEngineAdapter pushValue:2 state:state];
    [LSCEngineAdapter rawGet:state index:-2];

    if ([LSCEngineAdapter isNil:state index:-1])
    {
        [LSCEngineAdapter pop:state count:1];
        
        retValueCount = [exporter _instancePropertyWithSession:session
                                                      instance:instance
                                                typeDescriptor:typeDescriptor
                                                  propertyName:key];
    }

    //移除元表
    [LSCEngineAdapter remove:state index:-1-retValueCount];
    
    [exporter.context destroySession:session];
    
    
    
    return retValueCount;
}

/**
 *  对象销毁处理
 *
 *  @param state 状态机
 *
 *  @return 参数数量
 */
static int objectDestroyHandler (lua_State *state)
{
    if ([LSCEngineAdapter getTop:state] > 0 && [LSCEngineAdapter isUserdata:state index:1])
    {
        int index = [LSCEngineAdapter upvalueIndex:1];
        const void *ptr = [LSCEngineAdapter toPointer:state index:index];
        LSCExportsTypeManager *exporter = (__bridge LSCExportsTypeManager *)ptr;
        
        LSCSession *session = [exporter.context makeSessionWithState:state lightweight:NO];
        
        //如果为userdata类型，则进行释放
        LSCUserdataRef ref = (LSCUserdataRef)[LSCEngineAdapter toUserdata:state index:1];
        
        int errFuncIndex = [exporter.context catchLuaExceptionWithState:state queue:exporter.context.optQueue];
        
        [LSCEngineAdapter pushValue:1 state:state];
        [LSCEngineAdapter getField:state index:-1 name:"destroy"];
        if ([LSCEngineAdapter isFunction:state index:-1])
        {
            [LSCEngineAdapter pushValue:1 state:state];
            [LSCEngineAdapter pCall:state nargs:1 nresults:0 errfunc:errFuncIndex];
            [LSCEngineAdapter pop:state count:1];
        }
        else
        {
            [LSCEngineAdapter pop:state count:2]; //出栈方法、实例对象
        }
        
        //移除异常捕获方法
        [LSCEngineAdapter remove:state index:errFuncIndex];
        
        //释放内存
        CFBridgingRelease(ref -> value);
        
        [exporter.context destroySession:session];
    }
    
    return 0;
}


/**
 类型转换为字符串处理

 @param state 状态
 @return 参数数量
 */
static int classToStringHandler (lua_State *state)
{
    int index = [LSCEngineAdapter upvalueIndex:1];
    const void *ptr = [LSCEngineAdapter toPointer:state index:index];
    LSCExportsTypeManager *exporter = (__bridge LSCExportsTypeManager *)ptr;
    
    LSCSession *session = [exporter.context makeSessionWithState:state lightweight:NO];
    
    LSCExportTypeDescriptor *typeDescriptor = nil;
    
    [LSCEngineAdapter getField:state index:1 name:"_nativeType"];
    if ([LSCEngineAdapter type:state index:-1] == LUA_TLIGHTUSERDATA)
    {
        typeDescriptor = (__bridge LSCExportTypeDescriptor *)[LSCEngineAdapter toPointer:state index:-1];
    }
    
    if (typeDescriptor)
    {
        [LSCEngineAdapter pushString:[[NSString stringWithFormat:@"[%@ type]", typeDescriptor.typeName] UTF8String] state:state];
    }
    else
    {
        [session reportLuaExceptionWithMessage:@"can not describe unknown type."];
        [LSCEngineAdapter pushNil:state];
    }
    
    [exporter.context destroySession:session];
    
    return 1;
}

/**
 转换Prototype为字符串处理

 @param state 状态
 @return 参数数量
 */
static int prototypeToStringHandler (lua_State *state)
{
    int index = [LSCEngineAdapter upvalueIndex:1];
    const void *ptr = [LSCEngineAdapter toPointer:state index:index];
    LSCExportsTypeManager *exporter = (__bridge LSCExportsTypeManager *)ptr;
    
    LSCSession *session = [exporter.context makeSessionWithState:state lightweight:NO];
    
    LSCExportTypeDescriptor *typeDescriptor = nil;
    
    [LSCEngineAdapter getField:state index:1 name:"_nativeType"];
    if ([LSCEngineAdapter type:state index:-1] == LUA_TLIGHTUSERDATA)
    {
        typeDescriptor = (__bridge LSCExportTypeDescriptor *)[LSCEngineAdapter toPointer:state index:-1];
    }
    
    if (typeDescriptor)
    {
        [LSCEngineAdapter pushString:[[NSString stringWithFormat:@"[%@ prototype]", typeDescriptor.typeName] UTF8String] state:state];
    }
    else
    {
        [session reportLuaExceptionWithMessage:@"can not describe unknown prototype."];
        [LSCEngineAdapter pushNil:state];
    }
    
    [exporter.context destroySession:session];
    
    return 1;
}

/**
 设置原型的新属性处理

 @param state 状态
 @return 参数数量
 */
static int prototypeNewIndexHandler (lua_State *state)
{
    int index = [LSCEngineAdapter upvalueIndex:1];
    const void *ptr = [LSCEngineAdapter toPointer:state index:index];
    LSCExportsTypeManager *exporter = (__bridge LSCExportsTypeManager *)ptr;
    
    LSCSession *session = [exporter.context makeSessionWithState:state lightweight:YES];
    
    //t,k,v
    BOOL isPropertyReg = NO;
    if ([LSCEngineAdapter type:state index:3] == LUA_TTABLE)
    {
        //检测是否为属性设置
        LSCFunction *getter = nil;
        LSCFunction *setter = nil;
        
        [LSCEngineAdapter getField:state index:3 name:"get"];
        if ([LSCEngineAdapter type:state index:-1] == LUA_TFUNCTION)
        {
            LSCValue *getterValue = [LSCValue valueWithContext:exporter.context atIndex:-1];
            getter = [getterValue toFunction];
        }
        
        [LSCEngineAdapter pop:state count:1];
        
        [LSCEngineAdapter getField:state index:3 name:"set"];
        if ([LSCEngineAdapter type:state index:-1] == LUA_TFUNCTION)
        {
            LSCValue *setterValue = [LSCValue valueWithContext:exporter.context atIndex:-1];
            setter = [setterValue toFunction];
        }
        
        [LSCEngineAdapter pop:state count:1];
        
        if (getter || setter)
        {
            isPropertyReg = YES;
            
            //注册属性
            [LSCEngineAdapter getField:state index:1 name:"_nativeType"];
            if ([LSCEngineAdapter type:state index:-1] == LUA_TLIGHTUSERDATA)
            {
                LSCExportTypeDescriptor *typeDescriptor = (LSCExportTypeDescriptor *)[LSCEngineAdapter toPointer:state index:-1];
                
                LSCValue *propertyNameValue = [LSCValue valueWithContext:exporter.context atIndex:2];
                LSCExportPropertyDescriptor *propertyDescriptor = [[LSCExportPropertyDescriptor alloc] initWithName:[propertyNameValue toString] getterFunction:getter setterFunction:setter];
                
                NSMutableDictionary *properties = [typeDescriptor.properties mutableCopy];
                if (!typeDescriptor.properties)
                {
                    properties = [NSMutableDictionary dictionary];
                }
                [properties setObject:propertyDescriptor forKey:propertyDescriptor.name];
                typeDescriptor.properties = properties;
            }
        }
    }
    
    if (!isPropertyReg)
    {
        //直接设置
        [LSCEngineAdapter rawSet:state index:1];
    }
    
    [exporter.context destroySession:session];
    
    return 0;
}

/**
 *  对象转换为字符串处理
 *
 *  @param state 状态机
 *
 *  @return 参数数量
 */
static int objectToStringHandler (lua_State *state)
{
    int index = [LSCEngineAdapter upvalueIndex:1];
    const void *ptr = [LSCEngineAdapter toPointer:state index:index];
    LSCExportsTypeManager *exporter = (__bridge LSCExportsTypeManager *)ptr;
    
    LSCSession *session = [exporter.context makeSessionWithState:state lightweight:NO];
    
    LSCExportTypeDescriptor *typeDescriptor = nil;
    
    [LSCEngineAdapter getField:state index:1 name:"_nativeType"];
    if ([LSCEngineAdapter type:state index:-1] == LUA_TLIGHTUSERDATA)
    {
        typeDescriptor = (__bridge LSCExportTypeDescriptor *)[LSCEngineAdapter toPointer:state index:-1];
    }
    
    if (typeDescriptor)
    {
        NSString *desc = [NSString stringWithFormat:@"[%@ object<%p>]",
                          typeDescriptor.typeName, [LSCEngineAdapter toPointer:state index:1]];
        [LSCEngineAdapter pushString:[desc UTF8String] state:state];
    }
    else
    {
        [session reportLuaExceptionWithMessage:@"can not describe unknown object."];
        [LSCEngineAdapter pushNil:state];
    }
    
    [exporter.context destroySession:session];
    
    return 1;
}

/**
 *  子类化
 *
 *  @param state 状态机
 *
 *  @return 参数数量
 */
static int subClassHandler (lua_State *state)
{
    int index = [LSCEngineAdapter upvalueIndex:1];
    const void *ptr = [LSCEngineAdapter toPointer:state index:index];
    LSCExportsTypeManager *exporter = (__bridge LSCExportsTypeManager *)ptr;

    LSCSession *session = [exporter.context makeSessionWithState:state lightweight:NO];
    
    if ([LSCEngineAdapter type:state index:1] != LUA_TTABLE)
    {
        [session reportLuaExceptionWithMessage:@"please use the colon syntax to call the method"];
    }
    else if ([LSCEngineAdapter getTop:state] < 2 || [LSCEngineAdapter type:state index:2] != LUA_TSTRING)
    {
        [session reportLuaExceptionWithMessage:@"missing parameter subclass name or argument type mismatch.."];
    }
    else
    {
        //获取传入类型
        LSCExportTypeDescriptor *typeDescriptor = nil;
        [LSCEngineAdapter getField:state index:1 name:"_nativeType"];
        if ([LSCEngineAdapter type:state index:-1] == LUA_TLIGHTUSERDATA)
        {
            typeDescriptor = (__bridge LSCExportTypeDescriptor *)[LSCEngineAdapter toPointer:state index:-1];
        }
        [LSCEngineAdapter pop:state count:1];
        
        if (typeDescriptor)
        {
            //构建子类型描述
            NSString *typeName = [NSString stringWithUTF8String:[LSCEngineAdapter checkString:state index:2]];
            LSCExportTypeDescriptor *subTypeDescriptor = [[LSCExportTypeDescriptor alloc] initWithTypeName:typeName nativeType:typeDescriptor.nativeType];
            subTypeDescriptor.parentTypeDescriptor = typeDescriptor;
            [exportTypes setObject:subTypeDescriptor forKey:subTypeDescriptor.typeName];
            
            [exporter _exportsType:subTypeDescriptor state:state];
        }
        else
        {
            [session reportLuaExceptionWithMessage:@"can't subclass type! Invalid base type."];
        }
    }
    
    [exporter.context destroySession:session];
    
    return 0;
}

/**
 判断是否是该类型的子类
 
 @param state 状态机
 @return 参数数量
 */
static int subclassOfHandler (lua_State *state)
{
    int index = [LSCEngineAdapter upvalueIndex:1];
    const void *ptr = [LSCEngineAdapter toPointer:state index:index];
    LSCExportsTypeManager *exporter = (__bridge LSCExportsTypeManager *)ptr;
    
    BOOL flag = NO;
    LSCSession *session = [exporter.context makeSessionWithState:state lightweight:NO];
    
    if ([LSCEngineAdapter type:state index:1] != LUA_TTABLE)
    {
        [session reportLuaExceptionWithMessage:@"please use the colon syntax to call the method"];
    }
    else if ([LSCEngineAdapter getTop:state] < 2 || [LSCEngineAdapter type:state index:2] != LUA_TTABLE)
    {
        [session reportLuaExceptionWithMessage:@"missing parameter `type` or argument type mismatch."];
    }
    else
    {
        LSCExportTypeDescriptor *typeDescriptor = nil;
        if ([LSCEngineAdapter type:state index:1] == LUA_TTABLE)
        {
            [LSCEngineAdapter getField:state index:1 name:"_nativeType"];
            if ([LSCEngineAdapter type:state index:-1] == LUA_TLIGHTUSERDATA)
            {
                typeDescriptor = (__bridge LSCExportTypeDescriptor *)[LSCEngineAdapter toPointer:state index:-1];
            }
            [LSCEngineAdapter pop:state count:1];
        }
        
        LSCExportTypeDescriptor *checkTypeDescriptor = nil;
        if ([LSCEngineAdapter type:state index:2] == LUA_TTABLE)
        {
            [LSCEngineAdapter getField:state index:2 name:"_nativeType"];
            if ([LSCEngineAdapter type:state index:-1] == LUA_TLIGHTUSERDATA)
            {
                checkTypeDescriptor = (__bridge LSCExportTypeDescriptor *)[LSCEngineAdapter toPointer:state index:-1];
            }
            [LSCEngineAdapter pop:state count:1];
        }
        
        if (typeDescriptor && checkTypeDescriptor)
        {
            flag = [typeDescriptor subtypeOfType:checkTypeDescriptor];
        }
        else
        {
            [session reportLuaExceptionWithMessage:@"Unknown error."];
        }
    }
    
    [LSCEngineAdapter pushBoolean:flag state:state];
    [exporter.context destroySession:session];
    
    return 1;
}

/**
 判断是否是该类型的实例对象
 
 @param state 状态机
 @return 参数数量
 */
static int instanceOfHandler (lua_State *state)
{
    int index = [LSCEngineAdapter upvalueIndex:1];
    const void *ptr = [LSCEngineAdapter toPointer:state index:index];
    LSCExportsTypeManager *exporter = (__bridge LSCExportsTypeManager *)ptr;

    BOOL flag = NO;
    LSCSession *session = [exporter.context makeSessionWithState:state lightweight:NO];
    
    if ([LSCEngineAdapter getTop:state] < 2)
    {
        [session reportLuaExceptionWithMessage:@"missing parameter `type` or argument type mismatch."];
    }
    else
    {
        //获取实例类型
        LSCExportTypeDescriptor *typeDescriptor = nil;
        [LSCEngineAdapter getField:state index:1 name:"_nativeType"];
        if ([LSCEngineAdapter type:state index:-1] == LUA_TLIGHTUSERDATA)
        {
            typeDescriptor = (__bridge LSCExportTypeDescriptor *)[LSCEngineAdapter toPointer:state index:-1];
        }
        [LSCEngineAdapter pop:state count:1];
        
        if (typeDescriptor)
        {
            if ([LSCEngineAdapter type:state index:2] == LUA_TTABLE)
            {
                [LSCEngineAdapter getField:state index:2 name:"_nativeType"];
                if ([LSCEngineAdapter type:state index:-1] == LUA_TLIGHTUSERDATA)
                {
                    LSCExportTypeDescriptor *checkTypeDescriptor = (__bridge LSCExportTypeDescriptor *)[LSCEngineAdapter toPointer:state index:-1];
                    flag = [typeDescriptor subtypeOfType:checkTypeDescriptor];
                }
            }
        }
    }
    
    [LSCEngineAdapter pushBoolean:flag state:state];
    
    [exporter.context destroySession:session];

    return 1;
}

/**
 全局对象的index元方法处理

 @param state 状态
 @return 返回参数数量
 */
static int globalIndexMetaMethodHandler(lua_State *state)
{
    LSCExportsTypeManager *exporter = [LSCEngineAdapter toPointer:state index:[LSCEngineAdapter upvalueIndex:1]];
    LSCSession *session = [exporter.context makeSessionWithState:state lightweight:YES];
    
    //获取key
    NSString *key = [NSString stringWithUTF8String:[LSCEngineAdapter toString:state index:2]];
    
    [LSCEngineAdapter rawGet:state index:1];
    if ([LSCEngineAdapter isNil:state index:-1])
    {
        //检测是否该key是否为导出类型
        LSCExportTypeDescriptor *typeDescriptor = [LSCExportsTypeManager _getMappingTypeWithName:key];
        if (typeDescriptor)
        {
            //为导出类型
            [LSCEngineAdapter pop:state count:1];
            
            //先检测类型是否已经导入
            [LSCEngineAdapter pushString:typeDescriptor.typeName.UTF8String state:state];
            [LSCEngineAdapter rawGet:state index:1];
            
            if ([LSCEngineAdapter isNil:state index:-1])
            {
                [LSCEngineAdapter pop:state count:1];
                
                //导出类型
                [exporter _prepareExportsTypeWithDescriptor:typeDescriptor];
                
                //重新获取
                [LSCEngineAdapter pushString:typeDescriptor.typeName.UTF8String state:state];
                [LSCEngineAdapter rawGet:state index:1];
            }
        }
    }
    
    [exporter.context destroySession:session];
    
    return 1;
}

@end
