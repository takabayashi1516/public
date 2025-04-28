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

  static contain(a, b, extractionDiff = true) {
    // array case
    if (Array.isArray(a)) {
      if (!Array.isArray(b)) {
        if (extractionDiff) {
          return a;
        }
        return null;
      }
      const extractionArray = [];
      for (let idx = 0; idx < a.length; idx++) {
        const extraction = this.contain(a[idx], b[idx], extractionDiff);
        if (extraction) {
          extractionArray.push(extraction);
        }
      }
      return (extractionArray.length > 0) ? extractionArray : null;
    }
    // object case
    if ((typeof a === 'object') && (a !== null)) {
      if ((typeof b !== 'object') || (b === null)) {
        if (extractionDiff) {
          return a;
        }
        return null;
      }
      let extractionObject = {};
      Object.keys(a).forEach((k) => {
        if (!(k in b)) {
          if (extractionDiff) {
            extractionObject[k] = a[k];
          }
        }
        const extraction = this.contain(a[k], b[k], extractionDiff);
        if (extraction) {
          extractionObject[k] = extraction;
        }
      });
      return (Object.keys(extractionObject).length > 0) ? extractionObject : null;
    }
    // primitive case
    if (extractionDiff) {
      return (a === b) ? null : a;
    }
    return (a === b) ? a : null;
  }

  static arr2tbl(arr) {
    const tbl = [];
    if (!Array.isArray(arr)) {
      let row = {};
      this.#obj2tblelement_(arr, '', row);
      tbl.push(row);
      return tbl;
    }
    arr.forEach((elem, idx) => {
      let row = {};
      this.#obj2tblelement_(elem, '', row);
      tbl.push(row);
    });
    const header = [];
    tbl.forEach((r) => {
      Object.keys(r).forEach((c) => {
        if (header.indexOf(c) < 0) {
          header.push(c);
        }
      });
    });
    tbl.forEach((r) => {
      const clmns = Object.keys(r);
      header.forEach((fld) => {
        if (clmns.indexOf(fld) < 0) {
          r[fld] = null;
        }
      });
    });
    return tbl;
  }

  static #obj2tblelement_(objct, key, tblelement) {
    if (Array.isArray(objct)) {
      objct.forEach((v, i) => {
        const nkey = `${key}[${i}]`;
        this.#obj2tblelement_(v, nkey, tblelement);
      });
      return;
    }
    if ((typeof objct === 'object') && (objct !== null)) {
      for (let k in objct) {
        const nkey = `${key}.${k}`;
        this.#obj2tblelement_(objct[k], nkey, tblelement);
      }
      return;
    }
    tblelement[key] = objct;
  }

}
exports.Util = Util;
