project "ProjectCreator"
    kind "WindowedApp"
    language "C#"
    csversion "11"

    files
    {
        "src/**.cs"
    }

    links 
    {
        "System",
        "System.Windows.Forms",
        "System.Data",
        "System.Drawing",
        "System.Xml"
    }