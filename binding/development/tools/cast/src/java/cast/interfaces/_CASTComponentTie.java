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

public class _CASTComponentTie extends _CASTComponentDisp implements Ice.TieBase
{
    public
    _CASTComponentTie()
    {
    }

    public
    _CASTComponentTie(_CASTComponentOperations delegate)
    {
        _ice_delegate = delegate;
    }

    public java.lang.Object
    ice_delegate()
    {
        return _ice_delegate;
    }

    public void
    ice_delegate(java.lang.Object delegate)
    {
        _ice_delegate = (_CASTComponentOperations)delegate;
    }

    public boolean
    equals(java.lang.Object rhs)
    {
        if(this == rhs)
        {
            return true;
        }
        if(!(rhs instanceof _CASTComponentTie))
        {
            return false;
        }

        return _ice_delegate.equals(((_CASTComponentTie)rhs)._ice_delegate);
    }

    public int
    hashCode()
    {
        return _ice_delegate.hashCode();
    }

    public void
    beat(Ice.Current __current)
    {
        _ice_delegate.beat(__current);
    }

    public void
    configure(java.util.Map<java.lang.String, java.lang.String> config, Ice.Current __current)
    {
        _ice_delegate.configure(config, __current);
    }

    public void
    destroy(Ice.Current __current)
    {
        _ice_delegate.destroy(__current);
    }

    public String
    getID(Ice.Current __current)
    {
        return _ice_delegate.getID(__current);
    }

    public void
    run(Ice.Current __current)
    {
        _ice_delegate.run(__current);
    }

    public void
    setComponentManager(ComponentManagerPrx man, Ice.Current __current)
    {
        _ice_delegate.setComponentManager(man, __current);
    }

    public void
    setID(String id, Ice.Current __current)
    {
        _ice_delegate.setID(id, __current);
    }

    public void
    setTimeServer(TimeServerPrx ts, Ice.Current __current)
    {
        _ice_delegate.setTimeServer(ts, __current);
    }

    public void
    start(Ice.Current __current)
    {
        _ice_delegate.start(__current);
    }

    public void
    stop(Ice.Current __current)
    {
        _ice_delegate.stop(__current);
    }

    private _CASTComponentOperations _ice_delegate;
}
