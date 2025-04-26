const fs = require('fs');
const path = require('path');
const readdirp = require('readdirp');

class Util {

  static basename(name) {
    return path.basename(name);
  }

  static dirname(name) {
    return path.dirname(name);
  }

  static searchFiles(location, pattern) {
    return new Promise(function (resolve, reject) {
      let rc = [];
      readdirp(location, { fileFilter: pattern }).on('data', (entry) => {
        rc.push(entry);
      }).on('end', () => {
        resolve(rc);
      });
    });
  }

  static mkDirectory(name) {
    return new Promise(function (resolve, reject) {

      fs.mkdir(name, { recursive: true }, (err) => {
        if (err) {
          reject(err);
          return;
        }
        resolve(true);
      });

    });
  }

  static isDirectory(name) {
    return new Promise(function (resolve, reject) {
      fs.stat(name, (err, stats) => {
        if (err) {
          if (err.code === 'ENOENT') {
            resolve(null);
            return;
          }
          reject(err);
          return;
        }
        if (stats.isDirectory()) {
          resolve(true);
          return;
        }
        resolve(false);
      });
    });
  }

  static sleep(delayMs) {
    return new Promise(function (resolve, reject) {
      setTimeout(function () {
        resolve();
      }, delayMs);
    });
  }

  static contain(a, b, extractionLost = true) {
    // array case
    if (Array.isArray(a)) {
      if (!Array.isArray(b)) {
        if (extractionLost) {
          return a;
        }
        return null;
      }
      const extractionArray = [];
      for (let idx = 0; idx < a.length; idx++) {
        const extraction = this.contain(a[idx], b[idx], extractionLost);
        if (extraction) {
          extractionArray.push(extraction);
        }
      }
      return (extractionArray.length > 0) ? extractionArray : null;
    }
    // object case
    if ((typeof a === 'object') && (a !== null)) {
      if ((typeof b !== 'object') || (b === null)) {
        if (extractionLost) {
          return a;
        }
        return null;
      }
      let extractionObject = {};
      Object.keys(a).forEach((k) => {
        if (!(k in b)) {
          if (extractionLost) {
            extractionObject[k] = a[k];
          }
        }
        const extraction = this.contain(a[k], b[k], extractionLost);
        if (extraction) {
          extractionObject[k] = extraction;
        }
      });
      return (Object.keys(extractionObject).length > 0) ? extractionObject : null;
    }
    // primitive case
    if (extractionLost) {
      return (a === b) ? null : a;
    }
    return (a === b) ? a : null;
  }

}
exports.Util = Util;
