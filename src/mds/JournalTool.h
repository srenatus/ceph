// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*- 
// vim: ts=8 sw=2 smarttab

#include "mds/MDSUtility.h"
#include <vector>

// For Journaler::Header, can't forward-declare nested classes
#include <osdc/Journaler.h>

namespace librados {
  class IoCtx;
}

class LogEvent;

/**
 * Command line tool for investigating and repairing filesystems
 * with damaged metadata logs
 */
class JournalTool : public MDSUtility
{
    private:
    int rank;

    // Entry points
    int main_journal(std::vector<const char*> argv);
    int main_header(std::vector<const char*> argv);
    int main_event(std::vector<const char*> argv);

    // Shared functionality
    int recover_journal();

    // Journal operations
    int journal_inspect();

    // Header operations
    int header_set();


    public:

    void usage();
    JournalTool() :
      rank(0) {}
    ~JournalTool();
    void init();
    void shutdown();
    int main(std::vector<const char*> argv);
};


/**
 * A simple sequential reader for metadata journals.  Unlike
 * the MDS Journaler class, this is written to detect, record,
 * and read past corruptions and missing objects.  It is also
 * less efficient but more plainly written.
 */
class JournalScanner
{
  private:
  // Results of my scan
  bool header_present;
  bool header_valid;
  int const rank;
  int const pool_id;
  Journaler::Header *header;

  std::string obj_name(uint64_t offset) const;
  int scan_header();
  int scan_entries();

  public:
  JournalScanner(int rank_, int pool_id_) :
    header_present(false),
    header_valid(false),
    rank(rank_),
    pool_id(pool_id_),
    header(NULL) {};
  ~JournalScanner();

  int scan();
  int scan_header(librados::IoCtx &io);
  int scan_events(librados::IoCtx &io);

  // The results of the scan
  typedef std::pair<uint64_t, uint64_t> Range;
  typedef std::map<uint64_t, LogEvent*> EventMap;

  bool is_healthy() const;
  std::vector<std::string> objects_valid;
  std::vector<uint64_t> objects_missing;
  std::vector<Range> ranges_invalid;
  std::vector<uint64_t> events_valid;
  EventMap events;
};
