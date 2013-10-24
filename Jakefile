var njake = require('./njake'),
    path = require('path'),
    fs = require('fs'),
    ini = require('ini'),
    child_process = require('child_process');
    exec = child_process.exec;
    spawn = child_process.spawn;
    msbuild = njake.msbuild;

msbuild.setDefaults({
    properties: { Confiuguration: 'Release' },
    processor: 'x86',
    version: 'net4.0'
});

/**
 * Prepares the given template file by replacing all instances of the $(MG_SRC) placeholder token with
 * the MapGuide Open Source directory we're working with
 */
var prepareFile = function(templateFile, outputFile, callback) {
    if (!('MG_SRC' in process.env)) {
        callback('MG_SRC environment variable not defined. Set this to the root directory of your MapGuide source code (MgDev)');
    }
    var mgDir = process.env.MG_SRC;
    fs.readFile(templateFile, 'utf8', function(err, data) {
        if (err) {
            callback(err);
        } else {
            var mgDirNormal = path.normalize(mgDir);
            mgDirNormal = mgDirNormal.replace(/\\/g, '/');
            //console.log('Replacing placeholder with: ' + mgDirNormal);
            var result = data.replace(/\$\(MG_SRC\)/g, mgDirNormal);
            fs.writeFile(outputFile, result, 'utf8', function(err2) {
                callback(err2);
            });
        }
    });
};

desc('Build IMake');
task('imake', function() {
    console.log('[imake]: Build');
    msbuild({
        file: 'Oem/IMake/IMake.sln', 
        properties: {
            'Configuration':'Release',
            'Platform': 'Win32'
        },
        _parameters: [ '/nologo', '/v:q', '/clp:ErrorsOnly' ],
        targets: ['Build']
    }, function(code) {
        console.log('[imake]: Build success');
        complete();
    });
}, { async: true });

desc('Build SWIGEx');
task('swig', function() {
    console.log('[swig]: Build');
    msbuild({
        file: 'Oem/SWIGEx/SWIGEx.sln', 
        properties: {
            'Configuration':'Release',
            'Platform': 'Win32'
        },
        _parameters: [ '/nologo', '/v:q', '/clp:ErrorsOnly' ],
        targets: ['Build']
    }, function(code) {
        console.log('[swig]: Build success');
        complete();
    });
}, { async: true });

desc('Generate constants');
task('constants', ['imake'],  function() {
    console.log('[constants]: Preparing constants.xml');
    var templateFile = 'Constants.xml.templ';
    var outputFile = 'Constants.xml';
    prepareFile(templateFile, outputFile, function(err){
        if (err) {
            console.error('[constants]: Failed to prepare constants.xml: ' + err);
            fail();
        } else {
            console.log('[constants]: Running IMake');
            var dir = process.cwd();
            var exe = path.join(dir, 'Oem/IMake/Win32/IMake.exe');
            exec(exe + ' Constants.xml JavaScript constants.js', function(error, stdout, stderr) {
                if (error !== null) {
                    console.error('[constants]: exec error: ' + error);
                    fail();
                } else {
                    console.log('[constants]: IMake.exe completed');
                    complete();
                }
            });
        }
    });
}, { async: true });

desc('Generate SWIG wrapper');
task('apigen', ['swig', 'imake', 'constants'], function() {
    console.log('[apigen]: Preparing MapGuideApiGen.xml');
    var templateFile = 'MapGuideApiGen.xml.templ';
    var outputFile = 'MapGuideApiGen.xml';
    prepareFile(templateFile, outputFile, function(err){
        if (err) {
            console.error('[apigen]: Failed to prepare MapGuideApiGen.xml: ' + err);
            fail();
        } else {
            console.log('[apigen]: Running IMake');
            var dir = process.cwd();
            var exe = path.join(dir, 'Oem/IMake/Win32/IMake.exe');
            exec(exe + ' MapGuideApiGen.xml JavaScript', function(error, stdout, stderr) {
                if (error !== null) {
                    console.error('[apigen]: exec error: ' + error);
                    fail();
                } else {
                    console.log('[apigen]: IMake.exe completed');
                    complete();
                }
            });
        }
    });
}, { async: true });

/*
desc('Generate C++ source for node.js wrapper');
task('genwrapper', ['apigen'], function() {
    var dir = process.cwd();
    var exe = path.join(dir, 'Oem/SWIGEx/Win32/Swig.exe');
    var args = [
        '-c++',
        '-nodejs',
        '-catchallcode',
        'catchall.code',
        '-clsidcode',
        'getclassid.code',
        '-nodefault',
        '-lib',
        path.join(dir, 'Oem/SWIGEx/Lib'),
        '-o',
        path.join(dir, 'MgNodeJsApi/extension/MgApi_wrap.cpp'),
        path.join(dir, 'MapGuideApi.i')
    ];
    var cmd = exe + ' ' + args.join(' ');
    console.log('running: ' + cmd);
    exec(cmd, function(error, stdout, stderr) {
        if (error !== null) {
            console.error('exec error: ' + error);
            fail();
        } else {
            console.log('swig.exe completed');
            complete();
        }
    });
});
*/

desc('Build MapGuide API for node.js');
task('api', /*['genwrapper'],*/ function() {
    msbuild({
        file: 'MgNodeJsApi/node-extension.sln', 
        properties: {
            'Configuration':'Release',
            'Platform': 'Win32'
        },
        targets: ['Build']
    }, function(code) {
        complete();
    });
}, { async: true });

desc('Default target');
task('default', ['imake', 'swig', 'constants', 'apigen'], function() {
    console.log('Build complete');
});