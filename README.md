node-mapguide
=============

MapGuide API for node.js

Overview
--------

node-mapguide consists of the following components:

 * A pre-processor utility (IMake) to generate a constants.js file and to generate a sanitized SWIG input file
 * A heavily-modified version of SWIG (http://www.swig.org/) that supports generating Node.js wrapper code for MapGuide
 * A node native addon VS project to build the C++ sources generated by SWIG

Build Requirements
------------------

 * Microsoft Visual Studio 2012 (tested with Express Edition for Windows Desktop)
 * node.js source (tested on latest stable v0.10.21)
 * Jake (https://github.com/mde/jake)
 * njake (https://github.com/prabirshrestha/njake). A copy of njake is included here for convenience
 * A subversion checkout of MapGuide Open Source trunk

Build Instructions (Windows)
----------------------------

 1. Build MapGuide Open Source
 2. Build node.js from source. If you have multiple versions of Visual Studio, make sure node's build script uses the VS2012 compiler. MapGuide does not expose a C-linkable API thus node.js must be built with the same compiler used to build MapGuide.
 3. Install Jake locally: `npm install jake`
 4. Install ini module: `npm install ini`
 5. Build node-mapguide with jake from VS2012 command prompt
    * `jake MG_SRC=<path to MapGuide Source dir> NODE_SRC=<path to node.js source dir>`
    * `swig.bat` (HACK: This was supposed to be an exec() task in Jake, but some paths get screwed up. Anyone know why?)
    * `jake MG_SRC=<path to MapGuide Source dir> NODE_SRC=<path to node.js source dir> api`

Once built, the MapGuide API extension for node.js will reside in the `Release` directory of your node.js source

Build Instructions (Linux)
--------------------------

TBD

Using the extension in Node.js
------------------------------

```javascript
var mg = require("./MapGuideNodeJsApi");
mg.MgInitializeWebTier("C:\path\to\webconfig.ini");

//All MapGuide classes are prefixed under the alias you've required() under
var conn = new mg.MgSiteConnection();
var user = new mg.MgUserInformation("Anonymous", "");
conn.Open(user);
```

TODO
----

 * Use NAN (https://github.com/rvagg/nan) to better insulate us from changes to underlying Node API
 * Port existing API test runner across to node.js for basic API validation/verification