package org.elastos.tools.crosspl.utils

import java.lang.Exception

class Utils private constructor() {
    companion object {
        const val TAG : String = "crosspl"
    }

    open class CrossPLException(message: String?) : Exception(message)
}
