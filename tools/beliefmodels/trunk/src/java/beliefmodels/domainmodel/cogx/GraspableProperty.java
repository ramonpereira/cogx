// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// Ice version 3.3.1

package beliefmodels.domainmodel.cogx;

public class GraspableProperty extends ContinualFormula
{
    public GraspableProperty()
    {
        super();
    }

    public GraspableProperty(String id, float prob, ContinualStatus cstatus, boolean polarity, Graspable graspableValue)
    {
        super(id, prob, cstatus, polarity);
        this.graspableValue = graspableValue;
    }

    private static class __F implements Ice.ObjectFactory
    {
        public Ice.Object
        create(String type)
        {
            assert(type.equals(ice_staticId()));
            return new GraspableProperty();
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
        "::beliefmodels::adl::EpistemicObject",
        "::beliefmodels::adl::Formula",
        "::beliefmodels::domainmodel::cogx::ContinualFormula",
        "::beliefmodels::domainmodel::cogx::GraspableProperty",
        "::beliefmodels::domainmodel::cogx::SuperFormula",
        "::beliefmodels::domainmodel::cogx::UncertainSuperFormula"
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
        return __ids[4];
    }

    public String
    ice_id(Ice.Current __current)
    {
        return __ids[4];
    }

    public static String
    ice_staticId()
    {
        return __ids[4];
    }

    public void
    __write(IceInternal.BasicStream __os)
    {
        __os.writeTypeId(ice_staticId());
        __os.startWriteSlice();
        graspableValue.__write(__os);
        __os.endWriteSlice();
        super.__write(__os);
    }

    public void
    __read(IceInternal.BasicStream __is, boolean __rid)
    {
        if(__rid)
        {
            __is.readTypeId();
        }
        __is.startReadSlice();
        graspableValue = Graspable.__read(__is);
        __is.endReadSlice();
        super.__read(__is, true);
    }

    public void
    __write(Ice.OutputStream __outS)
    {
        Ice.MarshalException ex = new Ice.MarshalException();
        ex.reason = "type beliefmodels::domainmodel::cogx::GraspableProperty was not generated with stream support";
        throw ex;
    }

    public void
    __read(Ice.InputStream __inS, boolean __rid)
    {
        Ice.MarshalException ex = new Ice.MarshalException();
        ex.reason = "type beliefmodels::domainmodel::cogx::GraspableProperty was not generated with stream support";
        throw ex;
    }

    public Graspable graspableValue;
}
