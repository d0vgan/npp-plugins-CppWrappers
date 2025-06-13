JsonToolsCpp is a C++ wrapper (proxy) for the JsonToolsNppPlugin plugin written in C# by Mark Olson.  
  
Motivation:  
The .NET nature of JsonTools.dll makes Notepad++ to start 1-2 seconds longer. You may ask: who counts these seconds? You know, I do! As I don’t have Notepad++ constantly open (i.e. I open it to edit some files, then close, then open again, then close, etc.), the startup time does matter to me!  
So I was thinking: what if we could create some kind of C++ wrapper (proxy) around a C# plugin? Such C++ wrapper could allow a delayed load of a C# plugin - i.e. don’t load it initially (on startup) but load it on demand (when user explicitly calls the plugin’s functionality). This C++ wrapper should register the same menu items as the C# plugin it wraps, and then delegate actual execution to the C# plugin.  
This is how JsonToolsCpp was born.  
  
Installation:  
1. Go to "Notepad++\plugins" folder.  
2. Create a subfolder "JsonToolsCpp" there.  
3. Go to the just created folder "Notepad++\plugins\JsonToolsCpp".  
4. Put JsonToolsCpp.dll (this C++ plugin) and JsonTools.dll (the original C# plugin) there.  
5. You are done!  
  
Usage:  
JsonToolsCpp duplicates the menu items of the original JsonTools but does not load JsonTools.dll until you explicitly call one of its functions. Correspondingly, you don't have a delay on startup, but you also don't have JsonTools on Notepad++'s toolbar. This is by design.  
  
Links:  
* JsonTools sources: https://github.com/molsonkiko/JsonToolsNppPlugin  
* JsonTools binaries: https://github.com/molsonkiko/JsonToolsNppPlugin/releases  
