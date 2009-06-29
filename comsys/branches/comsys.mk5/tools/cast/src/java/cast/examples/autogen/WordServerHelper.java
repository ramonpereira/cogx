// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// Ice version 3.3.1

package cast.examples.autogen;

public final class WordServerHelper
{
    public static void
    write(Ice.OutputStream __outS, WordServer __v)
    {
        __outS.writeObject(__v);
    }

    public static void
    read(Ice.InputStream __inS, WordServerHolder __h)
    {
        __inS.readObject(__h.getPatcher());
    }
}
