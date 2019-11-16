package org.elastos.sdk.elephantwallet.contact.internal;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.annotations.SerializedName;

public class UserInfo extends HumanInfo implements IdentifyCodeInterface {
    public class UserJson extends HumanJson {
        @SerializedName(JsonKey.IdentifyCode)
        IdentifyCode identifyCode;
    }

    public static void SetCurrDevId(String devId) {
        mCurrDevId = devId;
    }

    public static String GetCurrDevId() {
        return mCurrDevId;
    }

    public String getCurrDevCarrierAddr() {
        for(CarrierInfo info: boundCarrierArray) {
            if(info.devInfo.devId.equals(mCurrDevId) == true) {
                return info.usrAddr;
            }
        }
        return null;
    }

    public String getCurrDevCarrierId() {
        for(CarrierInfo info: boundCarrierArray) {
            if(info.devInfo.devId.equals(mCurrDevId) == true) {
                return info.usrId;
            }
        }
        return null;
    }

    public int fromJson(String info) {
        UserJson json = new Gson().fromJson(info, UserJson.class);

        int ret = super.fromJsonObj(json);
        if(ret < 0) {
            return ret;
        }

        ret = identifyCode.fromJsonObj(json.identifyCode);
        if(ret < 0) {
            return ret;
        }

        return 0;
    }

    public String toJson() {
        UserJson json = new UserJson();
        json.identifyCode = new IdentifyCode();

        int ret = super.toJsonObj(json);
        if(ret < 0) {
            return null;
        }

        ret = this.identifyCode.toJsonObj(json.identifyCode);
        if(ret < 0) {
            return null;
        }

        Gson gson = new GsonBuilder().setPrettyPrinting().create();
        String val = gson.toJson(json);
        return val;
    }

    public IdentifyCode identifyCode = new IdentifyCode();

    private static String mCurrDevId;
} // class UserInfo

