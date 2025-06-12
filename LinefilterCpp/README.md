LinefilterCpp is a C++ wrapper (proxy) for the Linefilter3 plugin written in C# by seelisoft.  
  
Motivation:  
The .NET nature of Linefilter3.dll makes Notepad++ to start 1-2 seconds longer. You may ask: who counts these seconds? You know, I do! As I don’t have Notepad++ constantly open (i.e. I open it to edit some files, then close, then open again, then close, etc.), the startup time does matter to me!  
So I was thinking: what if we could create some kind of C++ wrapper (proxy) around a C# plugin? Such C++ wrapper could allow a delayed load of a C# plugin - i.e. don’t load it initially (on startup) but load it on demand (when user explicitly calls the plugin’s functionality). This C++ wrapper should register the same menu items as the C# plugin it wraps, and then delegate actual execution to the C# plugin.  
This is how LinefilterCpp was born.  
  
Installation:  
1. Go to "Notepad++\plugins" folder.  
2. Create a subfolder "LinefilterCpp" there.  
3. Go to the just created folder "Notepad++\plugins\LinefilterCpp".  
4. Put LinefilterCpp.dll (this C++ plugin) and Linefilter3.dll (the original C# plugin) there.  
5. You are done!  
  
Usage:  
LinefilterCpp duplicates the menu items of the original Linefilter3 but does not load Linefilter3.dll until you explicitly call one of its functions. Correspondingly, you don't have a delay on Notepad++ startup.  
  
Links:  
* Linefilter3 sources & binaries: https://www.seelisoft.net/Linefilter3/  
