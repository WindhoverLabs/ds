'use strict';

var path = require('path');

const CdrFlightAppsPlugin = require(path.join(global.CDR_INSTALL_DIR, '/commander/classes/CdrFlightAppsPlugin')).CdrFlightAppsPlugin;

module.exports = class CfeCdrFlightAppsPlugin extends CdrFlightAppsPlugin {
    constructor() {
        super(path.join(__dirname, 'web'));

        var content = {
            ds: {    		
                shortDescription: 'Data Storage',
                longDescription: 'Data Storage.',
                nodes: {
				    main: {
					    type: CdrFlightAppsPlugin.ContentType.LAYOUT,
					    shortDescription: 'DS Main',
					    longDescription: 'Data Storage Main.',
					    filePath: 'main_layout.lyt'
				    },
				    hk: {
					    type: CdrFlightAppsPlugin.ContentType.PANEL,
					    shortDescription: 'DS HK',
					    longDescription: 'Data Storage Housekeeping.',
					    filePath: 'hk.pug'
				    },
				    file_info: {
					    type: CdrFlightAppsPlugin.ContentType.PANEL,
					    shortDescription: 'DS File Info',
					    longDescription: 'Data Storage File Information.',
					    filePath: 'file_info.pug'
				    }
                }
            }
        };
        
      	this.addContent(content);
    };
};
