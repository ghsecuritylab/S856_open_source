// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "googleurl/src/gurl.h"
#include "net/base/registry_controlled_domain.h"
#include "testing/gtest/include/gtest/gtest.h"

#include "effective_tld_names_unittest1.cc"
#undef TOTAL_KEYWORDS
#undef MIN_WORD_LENGTH
#undef MAX_WORD_LENGTH
#undef MIN_HASH_VALUE
#undef MAX_HASH_VALUE
#include "effective_tld_names_unittest2.cc"

namespace net {
namespace {

class TestRegistryControlledDomainService :
    public RegistryControlledDomainService {
 public:
  // Sets the given data.
  static void UseDomainData(FindDomainPtr function) {
    RegistryControlledDomainService::UseFindDomainFunction(function);
  }

  // Creates a new dedicated instance to be used for testing, deleting any
  // previously-set one.
  static void UseDedicatedInstance() {
    delete static_cast<TestRegistryControlledDomainService*>(
        SetInstance(new TestRegistryControlledDomainService()));
  }

  // Restores RegistryControlledDomainService to using its default instance,
  // deleting any previously-set test instance.
  static void UseDefaultInstance() {
    delete static_cast<TestRegistryControlledDomainService*>(SetInstance(NULL));
  }
};

class RegistryControlledDomainTest : public testing::Test {
 protected:
  virtual void SetUp() {
    TestRegistryControlledDomainService::UseDedicatedInstance();
  }

  virtual void TearDown() {
    TestRegistryControlledDomainService::UseDefaultInstance();
  }
};


std::string GetDomainFromURL(const std::string& url) {
  return TestRegistryControlledDomainService::GetDomainAndRegistry(GURL(url));
}

std::string GetDomainFromHost(const std::wstring& host) {
  return TestRegistryControlledDomainService::GetDomainAndRegistry(host);
}

size_t GetRegistryLengthFromURL(const std::string& url,
                                bool allow_unknown_registries) {
  return TestRegistryControlledDomainService::GetRegistryLength(GURL(url),
      allow_unknown_registries);
}

size_t GetRegistryLengthFromHost(const std::wstring& host,
                                 bool allow_unknown_registries) {
  return TestRegistryControlledDomainService::GetRegistryLength(host,
      allow_unknown_registries);
}

bool CompareDomains(const std::string& url1, const std::string& url2) {
  GURL g1 = GURL(url1);
  GURL g2 = GURL(url2);
  return TestRegistryControlledDomainService::SameDomainOrHost(g1, g2);
}

TEST_F(RegistryControlledDomainTest, TestGetDomainAndRegistry) {
  TestRegistryControlledDomainService::UseDomainData(
      Perfect_Hash_Test1::FindDomain);

  // Test GURL version of GetDomainAndRegistry().
  EXPECT_EQ("baz.jp", GetDomainFromURL("http://a.baz.jp/file.html"));   // 1
  EXPECT_EQ("baz.jp.", GetDomainFromURL("http://a.baz.jp./file.html")); // 1
  EXPECT_EQ("", GetDomainFromURL("http://ac.jp"));                      // 2
  EXPECT_EQ("", GetDomainFromURL("http://a.bar.jp"));                   // 3
  EXPECT_EQ("", GetDomainFromURL("http://bar.jp"));                     // 3
  EXPECT_EQ("", GetDomainFromURL("http://baz.bar.jp"));                 // 3 4
  EXPECT_EQ("a.b.baz.bar.jp", GetDomainFromURL("http://a.b.baz.bar.jp"));
                                                                        // 4
  EXPECT_EQ("pref.bar.jp", GetDomainFromURL("http://baz.pref.bar.jp")); // 5
  EXPECT_EQ("b.bar.baz.com.", GetDomainFromURL("http://a.b.bar.baz.com."));
                                                                        // 6
  EXPECT_EQ("a.d.c", GetDomainFromURL("http://a.d.c"));                 // 7
  EXPECT_EQ("a.d.c", GetDomainFromURL("http://.a.d.c"));                // 7
  EXPECT_EQ("a.d.c", GetDomainFromURL("http://..a.d.c"));               // 7
  EXPECT_EQ("b.c", GetDomainFromURL("http://a.b.c"));                   // 7 8
  EXPECT_EQ("baz.com", GetDomainFromURL("http://baz.com"));             // none
  EXPECT_EQ("baz.com.", GetDomainFromURL("http://baz.com."));           // none

  EXPECT_EQ("", GetDomainFromURL(""));
  EXPECT_EQ("", GetDomainFromURL("http://"));
  EXPECT_EQ("", GetDomainFromURL("file:///C:/file.html"));
  EXPECT_EQ("", GetDomainFromURL("http://foo.com.."));
  EXPECT_EQ("", GetDomainFromURL("http://..."));
  EXPECT_EQ("", GetDomainFromURL("http://192.168.0.1"));
  EXPECT_EQ("", GetDomainFromURL("http://localhost"));
  EXPECT_EQ("", GetDomainFromURL("http://localhost."));
  EXPECT_EQ("", GetDomainFromURL("http:////Comment"));

  // Test std::wstring version of GetDomainAndRegistry().  Uses the same
  // underpinnings as the GURL version, so this is really more of a check of
  // CanonicalizeHost().
  EXPECT_EQ("baz.jp", GetDomainFromHost(L"a.baz.jp"));                 // 1
  EXPECT_EQ("baz.jp.", GetDomainFromHost(L"a.baz.jp."));               // 1
  EXPECT_EQ("", GetDomainFromHost(L"ac.jp"));                          // 2
  EXPECT_EQ("", GetDomainFromHost(L"a.bar.jp"));                       // 3
  EXPECT_EQ("", GetDomainFromHost(L"bar.jp"));                         // 3
  EXPECT_EQ("", GetDomainFromHost(L"baz.bar.jp"));                     // 3 4
  EXPECT_EQ("a.b.baz.bar.jp", GetDomainFromHost(L"a.b.baz.bar.jp"));   // 3 4
  EXPECT_EQ("pref.bar.jp", GetDomainFromHost(L"baz.pref.bar.jp"));     // 5
  EXPECT_EQ("b.bar.baz.com.", GetDomainFromHost(L"a.b.bar.baz.com.")); // 6
  EXPECT_EQ("a.d.c", GetDomainFromHost(L"a.d.c"));                     // 7
  EXPECT_EQ("a.d.c", GetDomainFromHost(L".a.d.c"));                    // 7
  EXPECT_EQ("a.d.c", GetDomainFromHost(L"..a.d.c"));                   // 7
  EXPECT_EQ("b.c", GetDomainFromHost(L"a.b.c"));                       // 7 8
  EXPECT_EQ("baz.com", GetDomainFromHost(L"baz.com"));                 // none
  EXPECT_EQ("baz.com.", GetDomainFromHost(L"baz.com."));               // none

  EXPECT_EQ("", GetDomainFromHost(L""));
  EXPECT_EQ("", GetDomainFromHost(L"foo.com.."));
  EXPECT_EQ("", GetDomainFromHost(L"..."));
  EXPECT_EQ("", GetDomainFromHost(L"192.168.0.1"));
  EXPECT_EQ("", GetDomainFromHost(L"localhost."));
  EXPECT_EQ("", GetDomainFromHost(L".localhost."));
}

TEST_F(RegistryControlledDomainTest, TestGetRegistryLength) {
  TestRegistryControlledDomainService::UseDomainData(
      Perfect_Hash_Test1::FindDomain);

  // Test GURL version of GetRegistryLength().
  EXPECT_EQ(2U, GetRegistryLengthFromURL("http://a.baz.jp/file.html", false));
                                                                        // 1
  EXPECT_EQ(3U, GetRegistryLengthFromURL("http://a.baz.jp./file.html", false));
                                                                        // 1
  EXPECT_EQ(0U, GetRegistryLengthFromURL("http://ac.jp", false));       // 2
  EXPECT_EQ(0U, GetRegistryLengthFromURL("http://a.bar.jp", false));    // 3
  EXPECT_EQ(0U, GetRegistryLengthFromURL("http://bar.jp", false));      // 3
  EXPECT_EQ(0U, GetRegistryLengthFromURL("http://baz.bar.jp", false));  // 3 4
  EXPECT_EQ(12U, GetRegistryLengthFromURL("http://a.b.baz.bar.jp", false));
                                                                        // 4
  EXPECT_EQ(6U, GetRegistryLengthFromURL("http://baz.pref.bar.jp", false));
                                                                        // 5
  EXPECT_EQ(11U, GetRegistryLengthFromURL("http://a.b.bar.baz.com", false));
                                                                        // 6
  EXPECT_EQ(3U, GetRegistryLengthFromURL("http://a.d.c", false));       // 7
  EXPECT_EQ(3U, GetRegistryLengthFromURL("http://.a.d.c", false));      // 7
  EXPECT_EQ(3U, GetRegistryLengthFromURL("http://..a.d.c", false));     // 7
  EXPECT_EQ(1U, GetRegistryLengthFromURL("http://a.b.c", false));       // 7 8
  EXPECT_EQ(0U, GetRegistryLengthFromURL("http://baz.com", false));     // none
  EXPECT_EQ(0U, GetRegistryLengthFromURL("http://baz.com.", false));    // none
  EXPECT_EQ(3U, GetRegistryLengthFromURL("http://baz.com", true));      // none
  EXPECT_EQ(4U, GetRegistryLengthFromURL("http://baz.com.", true));     // none

  EXPECT_EQ(std::string::npos, GetRegistryLengthFromURL("", false));
  EXPECT_EQ(std::string::npos, GetRegistryLengthFromURL("http://", false));
  EXPECT_EQ(std::string::npos,
            GetRegistryLengthFromURL("file:///C:/file.html", false));
  EXPECT_EQ(0U, GetRegistryLengthFromURL("http://foo.com..", false));
  EXPECT_EQ(0U, GetRegistryLengthFromURL("http://...", false));
  EXPECT_EQ(0U, GetRegistryLengthFromURL("http://192.168.0.1", false));
  EXPECT_EQ(0U, GetRegistryLengthFromURL("http://localhost", false));
  EXPECT_EQ(0U, GetRegistryLengthFromURL("http://localhost", true));
  EXPECT_EQ(0U, GetRegistryLengthFromURL("http://localhost.", false));
  EXPECT_EQ(0U, GetRegistryLengthFromURL("http://localhost.", true));
  EXPECT_EQ(0U, GetRegistryLengthFromURL("http:////Comment", false));

  // Test std::wstring version of GetRegistryLength().  Uses the same
  // underpinnings as the GURL version, so this is really more of a check of
  // CanonicalizeHost().
  EXPECT_EQ(2U, GetRegistryLengthFromHost(L"a.baz.jp", false));         // 1
  EXPECT_EQ(3U, GetRegistryLengthFromHost(L"a.baz.jp.", false));        // 1
  EXPECT_EQ(0U, GetRegistryLengthFromHost(L"ac.jp", false));            // 2
  EXPECT_EQ(0U, GetRegistryLengthFromHost(L"a.bar.jp", false));         // 3
  EXPECT_EQ(0U, GetRegistryLengthFromHost(L"bar.jp", false));           // 3
  EXPECT_EQ(0U, GetRegistryLengthFromHost(L"baz.bar.jp", false));       // 3 4
  EXPECT_EQ(12U, GetRegistryLengthFromHost(L"a.b.baz.bar.jp", false));  // 4
  EXPECT_EQ(6U, GetRegistryLengthFromHost(L"baz.pref.bar.jp", false));  // 5
  EXPECT_EQ(11U, GetRegistryLengthFromHost(L"a.b.bar.baz.com", false));
                                                                        // 6
  EXPECT_EQ(3U, GetRegistryLengthFromHost(L"a.d.c", false));            // 7
  EXPECT_EQ(3U, GetRegistryLengthFromHost(L".a.d.c", false));           // 7
  EXPECT_EQ(3U, GetRegistryLengthFromHost(L"..a.d.c", false));          // 7
  EXPECT_EQ(1U, GetRegistryLengthFromHost(L"a.b.c", false));            // 7 8
  EXPECT_EQ(0U, GetRegistryLengthFromHost(L"baz.com", false));          // none
  EXPECT_EQ(0U, GetRegistryLengthFromHost(L"baz.com.", false));         // none
  EXPECT_EQ(3U, GetRegistryLengthFromHost(L"baz.com", true));           // none
  EXPECT_EQ(4U, GetRegistryLengthFromHost(L"baz.com.", true));          // none

  EXPECT_EQ(std::string::npos, GetRegistryLengthFromHost(L"", false));
  EXPECT_EQ(0U, GetRegistryLengthFromHost(L"foo.com..", false));
  EXPECT_EQ(0U, GetRegistryLengthFromHost(L"..", false));
  EXPECT_EQ(0U, GetRegistryLengthFromHost(L"192.168.0.1", false));
  EXPECT_EQ(0U, GetRegistryLengthFromHost(L"localhost", false));
  EXPECT_EQ(0U, GetRegistryLengthFromHost(L"localhost", true));
  EXPECT_EQ(0U, GetRegistryLengthFromHost(L"localhost.", false));
  EXPECT_EQ(0U, GetRegistryLengthFromHost(L"localhost.", true));
}

TEST_F(RegistryControlledDomainTest, TestSameDomainOrHost) {
  TestRegistryControlledDomainService::UseDomainData(
      Perfect_Hash_Test2::FindDomain);

  EXPECT_TRUE(CompareDomains("http://a.b.bar.jp/file.html",
                             "http://a.b.bar.jp/file.html")); // b.bar.jp
  EXPECT_TRUE(CompareDomains("http://a.b.bar.jp/file.html",
                             "http://b.b.bar.jp/file.html")); // b.bar.jp
  EXPECT_FALSE(CompareDomains("http://a.foo.jp/file.html",    // foo.jp
                              "http://a.not.jp/file.html"));  // not.jp
  EXPECT_FALSE(CompareDomains("http://a.foo.jp/file.html",    // foo.jp
                              "http://a.foo.jp./file.html")); // foo.jp.
  EXPECT_FALSE(CompareDomains("http://a.com/file.html",       // a.com
                              "http://b.com/file.html"));     // b.com
  EXPECT_TRUE(CompareDomains("http://a.x.com/file.html",
                             "http://b.x.com/file.html"));    // x.com
  EXPECT_TRUE(CompareDomains("http://a.x.com/file.html",
                             "http://.x.com/file.html"));     // x.com
  EXPECT_TRUE(CompareDomains("http://a.x.com/file.html",
                             "http://..b.x.com/file.html"));  // x.com
  EXPECT_TRUE(CompareDomains("http://intranet/file.html",
                             "http://intranet/file.html"));   // intranet
  EXPECT_TRUE(CompareDomains("http://127.0.0.1/file.html",
                             "http://127.0.0.1/file.html"));  // 127.0.0.1
  EXPECT_FALSE(CompareDomains("http://192.168.0.1/file.html", // 192.168.0.1
                              "http://127.0.0.1/file.html")); // 127.0.0.1
  EXPECT_FALSE(CompareDomains("file:///C:/file.html",
                              "file:///C:/file.html"));       // no host
}

TEST_F(RegistryControlledDomainTest, TestDefaultData) {
  TestRegistryControlledDomainService::UseDefaultInstance();

  // Note that no data is set: we're using the default rules.
  EXPECT_EQ(3U, GetRegistryLengthFromURL("http://google.com", false));
  EXPECT_EQ(3U, GetRegistryLengthFromURL("http://stanford.edu", false));
  EXPECT_EQ(3U, GetRegistryLengthFromURL("http://ustreas.gov", false));
  EXPECT_EQ(3U, GetRegistryLengthFromURL("http://icann.net", false));
  EXPECT_EQ(3U, GetRegistryLengthFromURL("http://ferretcentral.org", false));
  EXPECT_EQ(0U, GetRegistryLengthFromURL("http://nowhere.foo", false));
  EXPECT_EQ(3U, GetRegistryLengthFromURL("http://nowhere.foo", true));
}

}  // namespace
}  // namespace net
