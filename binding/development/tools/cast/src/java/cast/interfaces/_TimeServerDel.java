// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// Ice version 3.3.0

package cast.interfaces;

public interface _TimeServerDel extends Ice._ObjectDel
{
    cast.cdl.CASTTime getCASTTime(java.util.Map<String, String> __ctx)
        throws IceInternal.LocalExceptionWrapper;

    cast.cdl.CASTTime fromTimeOfDayDouble(double todsecs, java.util.Map<String, String> __ctx)
        throws IceInternal.LocalExceptionWrapper;

    cast.cdl.CASTTime fromTimeOfDay(long secs, long usecs, java.util.Map<String, String> __ctx)
        throws IceInternal.LocalExceptionWrapper;

    void reset(java.util.Map<String, String> __ctx)
        throws IceInternal.LocalExceptionWrapper;
}
