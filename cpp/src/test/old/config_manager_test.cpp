// $Id$

#ifdef VISUAL_STUDIO
#include "stdafx.h"
#endif

#include <boost/test/unit_test.hpp>

#include "ConfigManager.h"

namespace cpp = pensar_digital::cpplib;

class Dummy
{
    private:
    std::string s;
    int i;
    std::vector<std::string> slist;

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_NVP(s);
        ar & BOOST_SERIALIZATION_NVP(i);
        ar & BOOST_SERIALIZATION_NVP(slist);
    }

    public:
    Dummy (std::string s_, int i_, const std::vector<std::string>& slist_): s(s_), i(i_), slist(slist_) {}
};

BOOST_AUTO_TEST_SUITE(config_manager_suite)

BOOST_AUTO_TEST_CASE(config_manager_test)
{
    //std::vector<std::string> v;
    //v.push_back ("a@a.com");
    //v.push_back ("b@a.com");
    //v.push_back ("c@a.com");
    //Dummy d ("teste", 1, v);

    cpp::ConfigManager<std::string> cfg;
    cfg.set ("p1", std::string("value1"));
    cfg.set ("p2", "2");
    //cfg.set ("p3", d);
    cfg.save ();

    cpp::ConfigManager<std::string> cfg2;
    cfg.load ();
    std::string p1 = cfg.get<std::string> ("p1");
    int p2 = cfg.get<int> ("p2");
    //Dummy p3 = cfg.get<Dummy> ("p3");

    BOOST_CHECK_EQUAL ("value1", p1);
    BOOST_CHECK_EQUAL (2, p2);
}

BOOST_AUTO_TEST_CASE(config_manager_xml_test)
{
    /*
    std::vector<std::string> v;
    v.push_back ("a@a.com");
    v.push_back ("b@a.com");
    v.push_back ("c@a.com");
    TestSerialization ("teste", 1, v);

    cpp::ConfigManager<std::string, boost::archive::xml_oarchive, boost::archive::xml_iarchive> cfg ("config.xml");
    cfg.set ("p1", "value1");
    cfg.set ("p2", "2");
    cfg.save ();

    cfg.set ("p1", "");
    cfg.set ("p2", "");
    cfg.load ();
    std::string p1 = cfg.get<std::string> ("p1");
    int p2 = cfg.get<int> ("p2");

    BOOST_CHECK_EQUAL ("value1", p1);
    BOOST_CHECK_EQUAL (2, p2);
    */
}

BOOST_AUTO_TEST_SUITE_END ()
