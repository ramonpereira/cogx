/*
 * @author:  Marko Mahnič
 * @created: jun 2010 
 *
 * © Copyright 2010 Marko Mahnič. 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef MATCHER_TZWIWD1K
#define MATCHER_TZWIWD1K

#include "Features.h"

namespace cogx { namespace vision {

class CSiftMatcherCudaSift: public CSiftMatcher
{
public:
   virtual void matchSiftDescriptors(TSiftVector& a, TSiftVector& b,
         TFeatureMatchVector& matches);
   virtual void matchSiftDescriptors(TSiftVector& a, std::vector<TSiftVector*>& bb,
         std::vector<TFeatureMatchVector*>& matches);
};

}} // namespace
#endif /* end of include guard: MATCHER_TZWIWD1K */
