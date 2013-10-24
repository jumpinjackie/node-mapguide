node-mapguide
=============

MapGuide API for node.js

Build Requirements
==================

 * Microsoft Visual Studio 2012 (tested with Express Edition for Windows Desktop)
 * node.js source (tested on latest stable v0.10.21)
 * Jake (https://github.com/mde/jake)
 * njake (https://github.com/prabirshrestha/njake). A copy of njake is included here for convenience
 * A subversion checkout of MapGuide Open Source trunk

Build Instructions (Windows)
============================

 1. Build MapGuide Open Source
 2. Build node.js from source. If you have multiple versions of Visual Studio, make sure node's build script uses the VS2012 compiler.
 3. Install Jake locally: `npm install jake`
 4. Install ini module: `npm install ini`
 5. Build node-mapguide with jake from VS2012 command prompt
    * `jake MG_SRC=<path to MapGuide Source dir> NODE_SRC=<path to node.js source dir>`
    * `swig.bat` (HACK: This was supposed to be an exec() task in Jake, but some paths get screwed up. Anyone know why?)
    * `jake MG_SRC=<path to MapGuide Source dir> NODE_SRC=<path to node.js source dir> api`

Once built, the MapGuide API extension for node.js will reside in the `Release` directory of your node.js source

Build Instructions (Linux)
==========================

TBD

Using the extension in Node.js
==============================

TBD