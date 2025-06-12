NppMenuSearchCpp is a C++ wrapper (proxy) for the NppMenuSearch plugin written in C# by Peter Frentrup.  
  
Motivation:  
The .NET nature of NppMenuSearch.dll makes Notepad++ to start 1-2 seconds longer. You may ask: who counts these seconds? You know, I do! As I don’t have Notepad++ constantly open (i.e. I open it to edit some files, then close, then open again, then close, etc.), the startup time does matter to me!  
So I was thinking: what if we could create some kind of C++ wrapper (proxy) around a C# plugin? Such C++ wrapper could allow a delayed load of a C# plugin - i.e. don’t load it initially (on startup) but load it on demand (when user explicitly calls the plugin’s functionality). This C++ wrapper should register the same menu items as the C# plugin it wraps, and then delegate actual execution to the C# plugin.  
This is how NppMenuSearchCpp was born.  
  
Installation:  
1. Go to "Notepad++\plugins" folder.  
2. Create a subfolder "NppMenuSearchCpp" there.  
3. Go to the just created folder "Notepad++\plugins\NppMenuSearchCpp".  
4. Put NppMenuSearchCpp.dll (this C++ plugin) and NppMenuSearch.dll (the original C# plugin) there.  
5. You are done!  
  
Usage:  
NppMenuSearchCpp duplicates the menu items of the original NppMenuSearch but does not load NppMenuSearch.dll until you explicitly call one of its functions. Correspondingly, you don't have a delay on startup, but you also don't have NppMenuSearch on Notepad++'s toolbar until you explicitly call one of NppMenuSearch's functions. This is by design.  
As a small side effect, NppMenuSearchCpp's menu item "Repeat previous command" is always enabled because NppMenuSearchCpp does not know whether NppMenuSearch already searched for something or not.  
  
Links:  
* NppMenuSearch sources: https://github.com/peter-frentrup/NppMenuSearch  
* NppMenuSearch binaries: https://sourceforge.net/projects/nppmenusearch/files/  
