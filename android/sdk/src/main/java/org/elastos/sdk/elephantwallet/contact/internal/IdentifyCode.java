package org.elastos.sdk.elephantwallet.contact.internal;

import android.support.annotation.CallSuper;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.annotations.SerializedName;

import java.util.HashMap;
import java.util.List;

class IdentifyCode implements IdentifyCodeInterface {
    @SerializedName(JsonKey.IdCodeMap)
    protected HashMap<Integer, String> idCodeMap;
    @SerializedName(JsonKey.UpdateTime)
    public long updateTime;

    public String phoneNumber;
    public String emailAddress;
    public String wechatId;
    public String remoteStorage;

    protected int fromJsonObj(IdentifyCode json) {
        this.idCodeMap = null;
        this.updateTime = json.updateTime;

        this.phoneNumber = json.phoneNumber;
        this.emailAddress = json.emailAddress;
        this.wechatId = json.wechatId;
        this.remoteStorage = json.remoteStorage;

        if(json.idCodeMap != null) {
            this.phoneNumber = json.idCodeMap.get(Type.PhoneNumber.id());
            this.emailAddress = json.idCodeMap.get(Type.EmailAddress.id());
            this.wechatId = json.idCodeMap.get(Type.WechatId.id());
            this.remoteStorage = json.idCodeMap.get(Type.RemoteStorage.id());
        }

        return 0;
    }

    protected int toJsonObj(IdentifyCode json) {
        json.idCodeMap = null;
        json.updateTime = this.updateTime;

        json.phoneNumber = this.phoneNumber;
        json.emailAddress = this.emailAddress;
        json.wechatId = this.wechatId;
        json.remoteStorage = this.remoteStorage;

        if(this.idCodeMap != null) {
            json.phoneNumber = this.idCodeMap.get(Type.PhoneNumber.id());
            json.emailAddress = this.idCodeMap.get(Type.EmailAddress.id());
            json.wechatId = this.idCodeMap.get(Type.WechatId.id());
            json.remoteStorage = this.idCodeMap.get(Type.RemoteStorage.id());
        }

        return 0;
    }

    IdentifyCode() {
    }
} // class Factory
