package cn.vimfung.luascriptcore;

import java.lang.annotation.ElementType;
import java.lang.annotation.Inherited;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * 导出类型配置
 * Created by vimfung on 2017/11/15.
 */
@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.TYPE)
@Deprecated
public @interface LuaExportTypeConfig
{
    /**
     * 排除的导出类方法名称列表
     *
     * @return 排除的导出类方法名称列表
     */
    public String[] excludeExportClassMethodNames() default {};

    /**
     * 排除的导出实例方法名称列表
     *
     * @return 排除的导出实例方法名称列表
     */
    public String[] excludeExportInstanceMethodsNames() default {};

    /**
     * 排除的导出属性名称列表
     *
     * @return 排除的导出属性名称列表
     */
    public String[] excludeExportFieldNames() default {};
}
