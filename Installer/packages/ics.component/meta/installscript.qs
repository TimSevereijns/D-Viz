function Component()
{
}

Component.prototype.createOperations = function()
{
    component.createOperations();

    if (systemInfo.productType === "windows") {
        component.addOperation("CreateShortcut",
            "@TargetDir@/D-Viz.exe", "@StartMenuDir@/D-Viz.lnk",
            "workingDirectory=@TargetDir@",
            "iconId=2", "description=Awesome World Executable");
    }
}
