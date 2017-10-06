/*
============================================================================
FMsearch: FM-Index Search
============================================================================
Copyright (C) 2017 Tobias Rausch

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
============================================================================
Contact: Tobias Rausch (rausch@embl.de)
============================================================================
*/

#ifndef NEIGHBORS_H
#define NEIGHBORS_H

#include <iostream>
#include <fstream>

#define BOOST_DISABLE_ASSERTS
#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/filesystem.hpp>
#include <boost/progress.hpp>

#include <sdsl/suffix_arrays.hpp>
#include <htslib/faidx.h>

using namespace sdsl;

namespace fmsearch {

inline void
reverseComplement(std::string& sequence) {
  std::string rev = boost::to_upper_copy(std::string(sequence.rbegin(), sequence.rend()));
  std::size_t i = 0;
  for(std::string::iterator revIt = rev.begin(); revIt != rev.end(); ++revIt, ++i) {
    switch (*revIt) {
    case 'A': sequence[i]='T'; break;
    case 'C': sequence[i]='G'; break;
    case 'G': sequence[i]='C'; break;
    case 'T': sequence[i]='A'; break;
    case 'N': sequence[i]='N'; break;
    default: break;
    }
  }
}

template<typename TAlphabet, typename TStringSet>
inline void
_neighbors(std::string const& query, TAlphabet const& alphabet, int32_t dist, bool indel, int32_t pos, TStringSet& strset) {
  for(int32_t i = pos; i < (int32_t) query.size();++i) {
    if (dist > 0) {
      if (indel) {
	// Insertion
	for(typename TAlphabet::const_iterator ait = alphabet.begin(); ait != alphabet.end(); ++ait) {
	  std::string ins("N");
	  ins[0] = *ait;
	  std::string newst = query.substr(0, i) + ins + query.substr(i);
	  _neighbors(newst, alphabet, dist - 1, indel, pos, strset);
	}
	// Deletion
	std::string newst = query.substr(0, i) + query.substr(i + 1);
	_neighbors(newst, alphabet, dist - 1, indel, pos + 1, strset);
      }
      for(typename TAlphabet::const_iterator ait = alphabet.begin(); ait != alphabet.end(); ++ait) {
	if (*ait != query[i]) {
	  std::string newst(query);
	  newst[i] = *ait;
	  _neighbors(newst, alphabet, dist - 1, indel, pos+1, strset);
	}
      }
    }
  }
  if ((indel) && (dist > 0)) {
    for(typename TAlphabet::const_iterator ait = alphabet.begin(); ait != alphabet.end(); ++ait) {
      std::string ins("N");
      ins[0] = *ait;
      std::string newst = query + ins;
      strset.insert(newst);
    }
  }
  strset.insert(query);
}
      

template<typename TAlphabet, typename TStringSet>
inline void
neighbors(std::string const& query, TAlphabet const& alphabet, int32_t dist, bool indel, TStringSet& strset) {
  _neighbors(query, alphabet, dist, indel, 0, strset);
}


}

#endif