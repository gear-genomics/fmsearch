/*
============================================================================
Silica: In-silico PCR
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

#ifndef JSON_H
#define JSON_H

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

namespace silica {


template<typename TPrimerBinds, typename TPrimerName, typename TPrimerSeq>
inline void
primerTxtOut(std::string const& outfile, faidx_t* fai, TPrimerBinds const& allp, TPrimerName const& pName, TPrimerSeq const& pSeq) {
  std::ofstream forfile(outfile.c_str());
  int32_t count = 0;
  for(typename TPrimerBinds::const_iterator it = allp.begin(); it != allp.end(); ++it, ++count) {
    forfile << "Primer_" << count << "_Tm="  << it->temp << std::endl;
    forfile << "Primer_" << count << "_Pos="  << std::string(faidx_iseq(fai, it->refIndex)) << ":" << it->pos << std::endl;
    forfile << "Primer_" << count << "_Ori=";
    if (it->onFor) forfile << "forward" << std::endl;
    else forfile << "reverse" << std::endl;
    forfile << "Primer_" << count << "_Name="  << pName[it->primerId] << std::endl;
    forfile << "Primer_" << count << "_MatchTm="  << it->perfTemp << std::endl;
    forfile << "Primer_" << count << "_Seq="  << pSeq[it->primerId] << std::endl;
    forfile << "Primer_" << count << "_Genome="  << it->genSeq << std::endl;
  }
  forfile.close();
}

template<typename TPcrProducts, typename TPrimerName, typename TPrimerSeq>
inline void
ampliconTxtOut(std::string const& outfile, faidx_t* fai, TPcrProducts const& pcrColl, TPrimerName const& pName, TPrimerSeq const& pSeq) {
  std::ofstream rfile(outfile.c_str());
  int32_t count = 0;
  for(typename TPcrProducts::const_iterator it = pcrColl.begin(); it != pcrColl.end(); ++it, ++count) {
    std::string chrom(faidx_iseq(fai, it->refIndex));
    rfile << "Amplicon_" << count << "_Length=" << it->leng << std::endl;
    rfile << "Amplicon_" << count << "_Penalty=" << it->penalty << std::endl;
    rfile << "Amplicon_" << count << "_For_Pos=" << chrom << ":" << it->forPos << std::endl;
    rfile << "Amplicon_" << count << "_For_Tm=" << it->forTemp << std::endl;
    rfile << "Amplicon_" << count << "_For_Name=" << pName[it->forId] << std::endl;
    rfile << "Amplicon_" << count << "_For_Seq=" << pSeq[it->forId] << std::endl;
    rfile << "Amplicon_" << count << "_Rev_Pos=" << chrom << ":" << it->revPos << std::endl;
    rfile << "Amplicon_" << count << "_Rev_Tm=" << it->revTemp << std::endl;
    rfile << "Amplicon_" << count << "_Rev_Name=" << pName[it->revId] << std::endl;
    rfile << "Amplicon_" << count << "_Rev_Seq=" << pSeq[it->revId] << std::endl;
    int32_t sl = -1;
    char* seq = faidx_fetch_seq(fai, chrom.c_str(), it->forPos, it->revPos, &sl);
    std::string seqstr = boost::to_upper_copy(std::string(seq));
    rfile << "Amplicon_" << count << "_Seq=" << seqstr << std::endl;
    free(seq);
  }
  rfile.close();
}

template<typename TPrimerBinds, typename TPrimerName, typename TPrimerSeq>
inline void
primerJsonOut(std::string const& outfile, faidx_t* fai, TPrimerBinds const& allp, TPrimerName const& pName, TPrimerSeq const& pSeq) {
  std::ofstream forfile(outfile.c_str());
  int32_t count = 0;
  forfile << "[" << std::endl;
  for(typename TPrimerBinds::const_iterator it = allp.begin(); it != allp.end(); ++it, ++count) {
    std::string chrom(faidx_iseq(fai, it->refIndex));
    if (count) forfile << "," << std::endl;
    forfile << "{\"Id\": " << count << ", ";
    forfile << "\"Tm\": "  << it->temp << ", ";
    forfile << "\"Chrom\": \"" << chrom << "\", ";
    forfile << "\"Pos\": "  << it->pos << ", ";
    forfile << "\"Ori\": \"";
    if (it->onFor) forfile << "forward" << "\", ";
    else forfile << "reverse" << "\", ";
    forfile << "\"Name\": \""  << pName[it->primerId] << "\", ";
    forfile << "\"MatchTm\": "  << it->perfTemp << ", ";
    forfile << "\"Seq\": \""  << pSeq[it->primerId] << "\", ";
    forfile << "\"Genome\": \""  << it->genSeq << "\"}";
  }
  forfile << std::endl;
  forfile << "]" << std::endl;
  forfile.close();
}
  
template<typename TPcrProducts, typename TPrimerName, typename TPrimerSeq>
inline void
ampliconJsonOut(std::string const& outfile, faidx_t* fai, TPcrProducts const& pcrColl, TPrimerName const& pName, TPrimerSeq const& pSeq) {
  std::ofstream rfile(outfile.c_str());
  int32_t count = 0;
  rfile << "[" << std::endl;
  for(typename TPcrProducts::const_iterator it = pcrColl.begin(); it != pcrColl.end(); ++it, ++count) {
    std::string chrom(faidx_iseq(fai, it->refIndex));
    if (count) rfile << "," << std::endl;
    rfile << "{\"Id\": " << count << ", ";
    rfile << "\"Length\": " << it->leng << ", ";
    rfile << "\"Penalty\": " << it->penalty << ", ";
    rfile << "\"Chrom\": \"" << chrom << "\", ";
    rfile << "\"ForPos\": " << it->forPos << ", ";
    rfile << "\"ForTm\": " << it->forTemp << ", ";
    rfile << "\"ForName\": \"" << pName[it->forId] << "\", ";
    rfile << "\"ForSeq\": \"" << pSeq[it->forId] << "\", ";
    rfile << "\"RevPos\": " << it->revPos << ", ";
    rfile << "\"RevTm\": " << it->revTemp << ", ";
    rfile << "\"RevName\": \"" << pName[it->revId] << "\", ";
    rfile << "\"RevSeq\": \"" << pSeq[it->revId] << "\", ";
    int32_t sl = -1;
    char* seq = faidx_fetch_seq(fai, chrom.c_str(), it->forPos, it->revPos, &sl);
    std::string seqstr = boost::to_upper_copy(std::string(seq));
    rfile << "\"Seq\": \"" << seqstr << "\"}";
    free(seq);
  }
  rfile << std::endl;
  rfile << "]" << std::endl;
  rfile.close();
}

}

#endif
