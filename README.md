node-mapguide
=============

MapGuide API for node.js

Build Requirements
==================

 * node.js source (tested on latest stable v0.10.21)
 * Jake (https://github.com/mde/jake)
 * njake (https://github.com/prabirshrestha/njake). A copy of njake is included here for convenience
 * A subversion checkout of MapGuide Open Source trunk

Build Instructions
==================

 1. Build MapGuide Open Source
 2. Build node.js from source
 3. Install Jake locally: npm install jake
 4. Install ini module: npm install ini
 5. Build node-mapguide with jake
    * jake
    * swig.bat
    * jake api

Once built, the MapGuide API extension for node.js will reside in the `Release` directory of your node.js source