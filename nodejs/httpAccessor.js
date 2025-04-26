const axios = require('axios');
const { Util } = require("./util.js");

class HttpAccessor {
  #retryParam_;

  constructor(retryParam) {
    this.#retryParam_ = retryParam;
  }

  #get1_(url, opt) {
    opt.proxy = false;
    return new Promise(function (resolve) {
      axios.get(url, opt).then(response => {
        return resolve({ error: null, response: response, body: response.data });
      }).catch(error => {
        if (!error) {
          return;
        }
        return resolve({ error: error, response: null, body: null });
      });
    });
  }

  #post1_(url, data, opt) {
    opt.proxy = false;
    return new Promise(function (resolve) {
      axios.post(url, data, opt).then(response => {
        return resolve({ error: null, response: response, body: response.data });
      }).catch(error => {
        if (!error) {
          return;
        }
        return resolve({ error: error, response: null, body: null });
      });
    });
  }

  #put1_(url, data, opt) {
    opt.proxy = false;
    return new Promise(function (resolve) {
      axios.put(url, data, opt).then(response => {
        return resolve({ error: null, response: response, body: response.data });
      }).catch(error => {
        if (!error) {
          return;
        }
        return resolve({ error: error, response: null, body: null });
      });
    });
  }

  #delete1_(url, opt) {
    opt.proxy = false;
    return new Promise(function (resolve) {
      axios.delete(url, opt).then(response => {
        return resolve({ error: null, response: response, body: response.data });
      }).catch(error => {
        if (!error) {
          return;
        }
        return resolve({ error: error, response: null, body: null });
      });
    });
  }

  async get(url, opt = {}) {
    const nmRetry = (this.#retryParam_.num) ? this.#retryParam_.num : 1;
    const secRetry = (this.#retryParam_.sec) ? this.#retryParam_.sec : 1;
    let error;
    for (let n = 0; n < nmRetry; n++) {
      const rslt = await this.#get1_(url, opt);
      if (rslt.error === null && rslt.response.status === 200) {
        return rslt.response;
      }
      error = rslt.error;
      console.error("stop http get retry",
          (error.response) ? error.response.status : error, n);
      await Util.sleep(secRetry * 1000);
    }
    throw error;
  }

  stream(url, opt, out) {
    opt.proxy = false;
    return new Promise(function (resolve) {
      opt.responseType = 'stream';
      axios.get(url, opt).then(response => {
        // console.log("set stream.");
        response.data.pipe(out);
        out.on("finish", () => {
          // console.log("stream finished.");
          return resolve(response);
        });
      }).catch(error => {
        if (!error) {
          return;
        }
        throw error;
      });
    });
  }

  async post(url, data, opt = {}) {
    const nmRetry = (this.#retryParam_.num) ? this.#retryParam_.num : 1;
    const secRetry = (this.#retryParam_.sec) ? this.#retryParam_.sec : 1;
    let error;
    for (let n = 0; n < nmRetry; n++) {
      const rslt = await this.#post1_(url, data, opt);
      if (rslt.error === null && rslt.response.status === 200) {
        return rslt.response;
      }
      error = rslt.error;
      console.error("stop http post retry",
          (error.response) ? error.response.status : error, n);
      await Util.sleep(secRetry * 1000);
    }
    throw error;
  }

  async put(url, data, opt = {}) {
    const nmRetry = (this.#retryParam_.num) ? this.#retryParam_.num : 1;
    const secRetry = (this.#retryParam_.sec) ? this.#retryParam_.sec : 1;
    let error;
    for (let n = 0; n < nmRetry; n++) {
      const rslt = await this.#put1_(url, data, opt);
      if (rslt.error === null && rslt.response.status === 200) {
        return rslt.response;
      }
      error = rslt.error;
      console.error("stop http put retry",
          (error.response) ? error.response.status : error, n);
      await Util.sleep(secRetry * 1000);
    }
    throw error;
  }

  async delete(url, opt = {}) {
    const nmRetry = (this.#retryParam_.num) ? this.#retryParam_.num : 1;
    const secRetry = (this.#retryParam_.sec) ? this.#retryParam_.sec : 1;
    let error;
    for (let n = 0; n < nmRetry; n++) {
      const rslt = await this.#delete1_(url, opt);
      if (rslt.error === null && rslt.response.status === 200) {
        return rslt.response;
      }
      error = rslt.error;
      console.error("stop http put retry",
          (error.response) ? error.response.status : error, n);
      await Util.sleep(secRetry * 1000);
    }
    throw error;
  }

}
exports.HttpAccessor = HttpAccessor;
