    static inline std::string version() {
        static auto pack = [] (const std::string& prop) {
            auto value = android::base::GetProperty(prop, "unknow");
            return android::base::StringPrintf("[%s]: [%s]\n", prop.c_str(), value.c_str());
        };

        std::string r;
        r += pack("ro.build.date");
        r += pack("ro.build.display.id");

        r += "--------\n";

        return r;
    }


// shell script



export LC_ALL=C

alias mk='make V=1 -j32 '
alias gdb='gdbclient.py'

smc() {
    if [[ -r $1 ]]; then
        java -jar ~/share/backup/smc/Smc.jar -graph -glevel 2 $1
        _1=$1
        _2=`basename $_1`
        dot -T pdf ${_1%.*}.dot -o ${_2%.*}.pdf
        code ${_2%.*}.pdf
        rm ${_1%.*}.dot
    fi
}

burnimg() {
    _burnimg() {
        FASTBOOT=`which fastboot`
        if [ -x $ANDROID_HOST_OUT/bin/fastboot ]; then
            FASTBOOT=$ANDROID_HOST_OUT/bin/fastboot
        fi

        while [ $# -ne 0 ];
        do
            if [ -f $OUT/$2 ]; then
                $FASTBOOT erase $1
                $FASTBOOT flash $1 $OUT/$2
            fi
            shift 2
        done
    }

    if [[ -r $1 && `file $1` =~ gzip ]]; then
        sudo mount tmpfs $OUT -t tmpfs -ouid=$UID,gid=$UID
        tar -zxvf $1 -C $OUT >/dev/null
    fi

    _burnimg \
        la_cache cache.img \
        la_log log.img \
        la_persist persist.img \
        la_system system.img \
        la_userdata userdata.img \
        la_vendor vendor.img \
        la_private private.img \

    sudo umount $OUT &>/dev/null

    unset -f _burnimg
}

stack() {
    if [[ -r $2 && `file $2` =~ gzip ]]; then
        sudo mount tmpfs $OUT/symbols -t tmpfs -ouid=$UID,gid=$UID
        tar -zxvf $2 -C $OUT >/dev/null
    fi

    echo "dump stack to `basename $1`.dump"
    $ANDROID_BUILD_TOP/development/scripts/stack $1 > `basename $1`.dump

    # _1=($_1=$(head -n 8 $1 | grep "<<<"))
    # aarch64-linux-android-objdump -s -D -C -S $OUT/symbols${_1[7]} >>`basename $1`.dump

    sudo umount $OUT/symbols &>/dev/null
}

setenv() {
    local ADB="adb wait-for-device"

    local VIN="HRYT2020TEST12117"
    local SN="HRYT20200623000425"

    _prepare() {
        $ADB root
        $ADB disable-verity
        $ADB reboot
        $ADB root
        $ADB remount

        # $ADB push bcmdhd.cal /persist/wifi/firmware/
        $ADB shell setprop persist.sys.timezone Asia/Shanghai
    }

    _setenv() {
        while [ $# -ne 0 ];
        do
            $ADB shell HHTConfigClient "sc/GT/$1"
            shift
        done
    }

    _verity() {
        $ADB root

        $ADB shell date `date "+%m%d%H%M%Y.%S"`
        $ADB shell HHTConfigClient sc/GT/token
        $ADB shell HHTConfigClient sc/GT/signalKey
    }

    if [ $# -ne 0 ]; then
        _prepare

        if [ $1 == "ee" ]; then
            _setenv \
                "HTTPS_URL_OPEN_GATEWAY https://iovcloud-test.human-horizons.com:30942" \
                "USE_TLS 1" \
                "HTTP_SERVERIP iovcloud-test.human-horizons.com" \
                "HTTP_PORT 31854" \
                "MQTT_ENV ee" \
                "MQTT_ENV2 ee" \
                "MQTT_SERVER 122.112.186.39:8885" \
                "MQTT_SERVER2 122.112.186.39:8884" \
                "VIN HRYT2020TEST12117" \
                "SN HRYT20200623000425"

        elif [ $1 == "pro" ]; then
            _setenv \
                "MQTT_ENV pro" \
                "MQTT_SERVER iovcloud-iot.human-horizons.com:8882" \
                "MQTT_ENV2 pro" \
                "MQTT_SERVER2 iovcloud-iot.human-horizons.com:8883" \
                "HTTPS_URL_OPEN_GATEWAY https://iovcloud-open2.human-horizons.com:443" \
                "USE_TLS 1" \
                "HTTP_SERVERIP iovcloud-idcm.human-horizons.com" \
                "HTTP_PORT 443" \
                "VIN HHTESTCAR00000001" \
                "SN IDCMSNTEST000001"

        elif [ $1 == "test" ]; then
            _setenv
                "HTTPS_URL_OPEN_GATEWAY https://iovcloud-test.human-horizons.com:31358" \
                "USE_TLS 0" \
                "HTTP_SERVERIP iovcloud-test.human-horizons.com" \
                "HTTP_PORT 36977" \
                "MQTT_ENV dev" \
                "MQTT_SERVER 122.112.186.39:1883" \
                "MQTT_ENV2 integration" \
                "MQTT_SERVER2 122.112.186.39:1883" \
                "VIN ZDF20200330000010" \
                "SN SNSN1202HRYT00017"
        else
            sleep 1
        fi
    fi

    _verity

    unset -f _prepare _setenv _verity
}

if [ -f build/envsetup.sh ]; then
    source build/envsetup.sh
    if [[ `pwd | grep HA1` ]]; then
        lunch cip-userdebug
    fi
    if [[ `pwd | grep IDCM` ]]; then
        lunch gin-userdebug
    fi
    if [[ `pwd | grep FSEM` ]]; then
        lunch gin-userdebug
    fi
fi
    
