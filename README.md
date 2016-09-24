# Visual Studio Project Files for Autodesk DWF Toolkit

Here you can find DWF Toolkit 7.7 updated for Visual Studio 2012, 2013 and 2015.
The changes are minimal and fall into 2 categories:

- New project files

  These were created by upgrading older project files with
  new versions of Visual Studio, with a few small changes to output folders.
  
- Warning fixes

  DWF Toolkit was originally configured to treat warnings as errors
  and each new version of Visual Studio found new things to complain about.

I only checked "Read-Write WChar DLL" build configuration and only in Visual Studio 2010
and higher, but other configurations and platforms should still work, unless they were
already broken.

If you use code from this respository you have to comply with the original
[Autodesk license for DWF Toolkit 7.7](http://usa.autodesk.com/adsk/servlet/item?siteID=123112&id=5522878).
