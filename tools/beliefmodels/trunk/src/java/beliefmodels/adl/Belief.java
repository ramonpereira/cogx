// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// Ice version 3.3.1

package beliefmodels.adl;

public class Belief extends EpistemicObject
{
    public Belief()
    {
        super();
    }

    public Belief(String id, SpatioTemporalFrame sigma, AgentStatus ags, Formula phi, cast.cdl.CASTTime timeStamp)
    {
        super(id);
        this.sigma = sigma;
        this.ags = ags;
        this.phi = phi;
        this.timeStamp = timeStamp;
    }

    private static class __F implements Ice.ObjectFactory
    {
        public Ice.Object
        create(String type)
        {
            assert(type.equals(ice_staticId()));
            return new Belief();
        }

        public void
        destroy()
        {
        }
    }
    private static Ice.ObjectFactory _factory = new __F();

    public static Ice.ObjectFactory
    ice_factory()
    {
        return _factory;
    }

    public static final String[] __ids =
    {
        "::Ice::Object",
        "::beliefmodels::adl::Belief",
        "::beliefmodels::adl::EpistemicObject"
    };

    public boolean
    ice_isA(String s)
    {
        return java.util.Arrays.binarySearch(__ids, s) >= 0;
    }

    public boolean
    ice_isA(String s, Ice.Current __current)
    {
        return java.util.Arrays.binarySearch(__ids, s) >= 0;
    }

    public String[]
    ice_ids()
    {
        return __ids;
    }

    public String[]
    ice_ids(Ice.Current __current)
    {
        return __ids;
    }

    public String
    ice_id()
    {
        return __ids[1];
    }

    public String
    ice_id(Ice.Current __current)
    {
        return __ids[1];
    }

    public static String
    ice_staticId()
    {
        return __ids[1];
    }

    public void
    __write(IceInternal.BasicStream __os)
    {
        __os.writeTypeId(ice_staticId());
        __os.startWriteSlice();
        __os.writeObject(sigma);
        __os.writeObject(ags);
        __os.writeObject(phi);
        timeStamp.__write(__os);
        __os.endWriteSlice();
        super.__write(__os);
    }

    private class Patcher implements IceInternal.Patcher
    {
        Patcher(int member)
        {
            __member = member;
        }

        public void
        patch(Ice.Object v)
        {
            try
            {
                switch(__member)
                {
                case 0:
                    __typeId = "::beliefmodels::adl::SpatioTemporalFrame";
                    sigma = (SpatioTemporalFrame)v;
                    break;
                case 1:
                    __typeId = "::beliefmodels::adl::AgentStatus";
                    ags = (AgentStatus)v;
                    break;
                case 2:
                    __typeId = "::beliefmodels::adl::Formula";
                    phi = (Formula)v;
                    break;
                }
            }
            catch(ClassCastException ex)
            {
                IceInternal.Ex.throwUOE(type(), v.ice_id());
            }
        }

        public String
        type()
        {
            return __typeId;
        }

        private int __member;
        private String __typeId;
    }

    public void
    __read(IceInternal.BasicStream __is, boolean __rid)
    {
        if(__rid)
        {
            __is.readTypeId();
        }
        __is.startReadSlice();
        __is.readObject(new Patcher(0));
        __is.readObject(new Patcher(1));
        __is.readObject(new Patcher(2));
        timeStamp = new cast.cdl.CASTTime();
        timeStamp.__read(__is);
        __is.endReadSlice();
        super.__read(__is, true);
    }

    public void
    __write(Ice.OutputStream __outS)
    {
        Ice.MarshalException ex = new Ice.MarshalException();
        ex.reason = "type beliefmodels::adl::Belief was not generated with stream support";
        throw ex;
    }

    public void
    __read(Ice.InputStream __inS, boolean __rid)
    {
        Ice.MarshalException ex = new Ice.MarshalException();
        ex.reason = "type beliefmodels::adl::Belief was not generated with stream support";
        throw ex;
    }

    public SpatioTemporalFrame sigma;

    public AgentStatus ags;

    public Formula phi;

    public cast.cdl.CASTTime timeStamp;
}
