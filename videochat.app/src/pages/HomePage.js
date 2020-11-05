"use strict";
process.env.CAFUI2 = true;

const Page = require("yunos/page/Page");
const CompositeView = require("yunos/ui/view/CompositeView");
const Power = require("yunos/device/Power");
const power = Power.getInstance();
const HomeController = require("../controllers/HomeController");
const Resource = require("yunos/content/resource/Resource");
const resource = Resource.getInstance();
global.getString = (key, options) => {
    if (options) {
        return resource.getString(key, options);
    }
    return resource.getString(key);
};
global.getImageSrc = (key) => {
    return resource.getImageSrc(key);
};

const TAG = "HomePage KONTAR";
class HomePage extends Page {

    constructor() {
        super();
    }

    onStart() {
        this.init();
        this.mWakeLock = power.newWakeLock(Power.WakeLockLevel.BRIGHT, "VideoChat");
        this.mWakeLock.acquire();
    }

    onBackKey() {
        return this.mHomeController.onBackKey();
    }

    onStop() {
        // Zhenyu
        this.mHomeController.onStop();
        this.mWakeLock.release();
    }

    init() {
        let homeController = this.mHomeController = new HomeController(this);
        this.window.addChild(homeController);
    }
}
module.exports = HomePage;
