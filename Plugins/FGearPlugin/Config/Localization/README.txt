FGear 编辑器双语本地化

使用插件
--------
FGearPlugin.uplugin 中的 LocalizationTargets 声明负责加载 Content/Localization/FGearEditor。
加载策略为 Editor；中文编辑器使用 zh-Hans 资源。复制并启用整个插件即可使用。
无需宿主项目中的 LocalizationPaths、FGearEditor 控制板目标或本地化生成配置。
此目标不会自动出现在 UE 本地化控制板中。

修改现有译文
------------
编辑插件 Content/Localization/FGearEditor/zh-Hans/FGearEditor.po，然后在 PowerShell 中执行：

  & '<插件目录>/Scripts/Localize-FGear.ps1' -Action Import,Compile,Report

只重新编译当前 archive：

  & '<插件目录>/Scripts/Localize-FGear.ps1' -Action Compile

源码新增或修改字段后
------------------
  & '<插件目录>/Scripts/Localize-FGear.ps1' -Action Gather,Export

在导出的 PO 中补齐或审阅译文，再执行 Import,Compile,Report。
已有相同键和原文的译文会保留；原文变化的条目需要重新审阅。
采集可能包含内部属性及函数参数，翻译范围仍以 FGear 细节面板字段为准。

指定宿主工程和引擎
------------------
脚本默认向上查找宿主 .uproject，并通过 EngineAssociation 查找本机引擎。
安装在引擎目录、存在多个宿主工程或无法解析引擎关联时，显式传入：

  & '<插件目录>/Scripts/Localize-FGear.ps1' -Project '<宿主工程路径>.uproject' -EngineRoot '<引擎根目录>' -Action Import,Compile,Report

宿主工程必须能够启用当前插件。模板中的路径由脚本按插件实际位置展开，
不依赖插件位于工程目录还是引擎目录，也不依赖工程名称或盘符。
脚本仅支持 Windows，使用对应引擎的 UnrealEditor-Cmd.exe。

格式与维护
----------
字段名称：英文（中文）。描述：完整英文原文，换行追加中文。
保留命名空间、属性键、英文原文、Category 中的 | 层级分隔符。
Events 是引擎共享分类，沿用引擎中文“事件”，不要改成双语以影响其他插件。
不要通过修改 C++ 成员名实现翻译。

.ini.in 是模板，不能直接传给 GatherText。脚本生成的绝对路径 INI 位于插件 Intermediate。
日志、报告以及 Gather/Import 前的自动备份位于插件 Saved/Localization/FGearEditor。
这些生成内容不需要随插件提交。应提交 uplugin、模板、脚本及 Content/Localization 中的资源。
使用 PO 文件前先关闭旧的翻译编辑窗口，避免旧会话保存时覆盖磁盘上的新译文。
编译后重启 UE，确保当前编辑器会话加载最新资源。
