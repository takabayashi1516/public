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

  static contain(a, b) {
    // array case
    if (Array.isArray(a)) {
      if (!Array.isArray(b)) {
        return a;
      }
      const missingArray = [];
      for (let idx = 0; idx < a.length; idx++) {
        const miss = this.contain(a[idx], b[idx]);
        if (miss) {
          missingArray.push(miss);
        }
      }
      return (missingArray.length > 0) ? missingArray : null;
    }
    // object case
    if ((typeof a === 'object') && (a !== null)) {
      if ((typeof b !== 'object') || (b === null)) {
        return a;
      }
      let missingObject = {};
      Object.keys(a).forEach((k) => {
        if (!(k in b)) {
          missingObject[k] = a[k];
        }
        const miss = this.contain(a[k], b[k]);
        if (miss) {
          missingObject[k] = miss;
        }
      });
      return (Object.keys(missingObject).length > 0) ? missingObject : null;
    }
    // primitive case
    if (a === b) {
      return null;
    }
    return a;
  }

}
exports.Util = Util;
