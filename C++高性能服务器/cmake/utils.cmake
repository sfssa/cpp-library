# 强制重新定义源文件__FILE__宏，将其替换为源文件的相对路径
# 声明一个cmake函数，接受一个参数targetname
function(force_redefine_file_macro_for_sources targetname)
    # 获取targetname的指定源文件列表，然后存储到source_files中
    get_target_property(source_files "${targetname}" SOURCES)
    # 遍历列表中每个文件
    foreach(sourcefile ${source_files})
        # 获取绝对路径存放在defs中
        get_property(defs SOURCE "${sourcefile}"
            PROPERTY COMPILE_DEFINITIONS)
        # 获取相对路径并用相对路径替换filename中项目目录部分
        get_filename_component(filepath "${sourcefile}" ABSOLUTE)
        string(REPLACE ${PROJECT_SOURCE_DIR}/ "" relpath ${filepath})
        # 将__FILE__定义添加到defs列表中，使用相对路径
        list(APPEND defs "__FILE__=\"${relpath}\"")
        # 设置源文件的编译定义为更新后的defs列表，重新定义__FILE__宏
        set_property(
            SOURCE "${sourcefile}"
            PROPERTY COMPILE_DEFINITIONS ${defs}
            )
    endforeach() # 结束循环
endfunction()