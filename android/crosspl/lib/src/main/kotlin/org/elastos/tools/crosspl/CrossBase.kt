package org.elastos.tools.crosspl

import android.util.Log
import org.elastos.tools.crosspl.annotation.CrossClass
import org.elastos.tools.crosspl.annotation.CrossInterface
import org.elastos.tools.crosspl.utils.Utils
import kotlin.reflect.KClass
import kotlin.reflect.full.primaryConstructor

@CrossClass
open class CrossBase protected constructor(
            val className: String,
            var nativeHandle: Long = 0)
{

    init {
        if(nativeHandle == 0L) {
            Log.d(Utils.TAG, "CrossBase CreateNativeObject() className=$className")
            nativeHandle = CreateNativeObject(className)
        }
        Log.i(Utils.TAG, "construct " + toString())
        if(nativeHandle <= 0L) {
            throw Utils.CrossPLException("Failed to create native object.")
        }
    }

    open fun finalize() {
        DestroyNativeObject(className, nativeHandle)
        Log.i(Utils.TAG, "deconstruct " + toString())
    }

    open fun bind() {
        bindPlatformHandle(this)
    }

    open fun unbind() {
        unbindPlatformHandle(this)
    }

    @CrossInterface
    override fun toString(): String {
        return "${this.javaClass.name}{nativeHandle=${nativeHandle}}"
    }

    @CrossInterface
    private external fun bindPlatformHandle(thisObj: CrossBase)

    @CrossInterface
    private external fun unbindPlatformHandle(thisObj: CrossBase)


//    private var nativeHandle: Long = 0
    private companion object {
        init {
            Log.d(Utils.TAG, "Loading libcrosspl.so ...")
            System.loadLibrary("crosspl")
            Log.d(Utils.TAG, "Loaded libcrosspl.so ...")
        }

        @CrossInterface
        private external fun CreateNativeObject(className: String): Long

        @CrossInterface
        private external fun DestroyNativeObject(className: String, nativeHandle: Long)

//        @CrossInterface
//        private external fun BindPlatformObject(obj: CrossBase): Long
//
//        @CrossInterface
//        private external fun UnbindPlatformObject(obj: CrossBase): Long
    }
}
