'use strict';

var path = require('path');

const CdrPlugin = require(path.join(global.CDR_INSTALL_DIR, '/commander/classes/CdrPlugin')).CdrPlugin;

module.exports = class CfeCdrPlugin extends CdrPlugin {
  constructor(urlBase) {
    super('ds', path.join(__dirname, 'web', urlBase));
  }

  getContent() {
    var result = {
      shortDescription: 'Data Storage',
      longDescription: 'Data Storage.',
      nodes: {
				main: {
					type: CdrPlugin.ContentType.LAYOUT,
					shortDescription: 'DS Main',
					longDescription: 'Data Storage Main.',
					filePath: '/main_layout.lyt'
				},
				hk: {
					type: CdrPlugin.ContentType.PANEL,
					shortDescription: 'DS HK',
					longDescription: 'Data Storage Housekeeping.',
					filePath: '/hk.pug'
				},
				file_info: {
					type: CdrPlugin.ContentType.PANEL,
					shortDescription: 'DS File Info',
					longDescription: 'Data Storage File Information.',
					filePath: '/file_info.pug'
				}
      }
    };

    return result;
  }
};

//var plugin = new CfeCdrPlugin();

//let CfeCdrPlugin = new class extends CdrPlugin {
