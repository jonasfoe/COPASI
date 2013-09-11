#ifndef COPASI_COptLogItem
#define COPASI_COptLogItem

#include <string>
#include <sstream>
#include <vector>
#include <ctime>

#include "copasi.h"

class COptLogItem
  {
  // Attributes
  public:
    /**
     * Enumeration of IDs for all messages
     */
    enum MsgID
    {
      STD_start,
      STD_start_nodoc,
      STD_early_stop,
      STD_finish_x_of_max_iter,
      Std_finish_x_of_max_gener,
      STD_finish_temp_info,
      STD_initial_point_out_of_domain,

      PS_usrdef_error_swarm_size,
      PS_info_informants,
      PS_no_particle_improved,
      PS_stddev_below_tol,

      SA_steps_per_temp,
      SA_fval_progress_lower_that_tol,
      SA_fval_tol_termination,

      DE_usrdef_error_pop_size,
      DE_fittest_not_changed_x_random_generated,

      GA_fittest_not_changed_x_random_generated,

      GASR_usrdef_error_pf,
      GASR_fittest_not_changed_x_random_generated,

    };

    /**
     * Vector of message headers for all defined MsgIDs
     */
    static const std::string MsgIDHeader[];

    /**
     * Vector of message subtexts for all defined MsgIDs
     */
    static const std::string MsgIDSubtext[];

  protected:
    /**
     * Message ID for this log item.
     */
    MsgID mID;

    /**
     * Timestamp for this log item.
     */
    time_t mTimestamp;

    /**
     * Iteration that this log item was created at (optional).
     */
    unsigned C_INT32 mIteration;

    /**
     * All further variables fed into this log item (optional).
     */
    std::vector<std::string> mMsgVars;

    /**
     * A string containing further detailed information of the current state of the optimization method (optional).
     */
    std::string mStatusDump;

  private:

    // Operations
  private:

  protected:
    /**
     * Replace %s%, %_timestamp% and %_iteration% in strings.
     * @param const std::string & string template
     * @param unsigned C_Int32 * current position in mMsgVars
     * @return std::string filledString
     */
    std::string fillString(const std::string & str, unsigned C_INT32 * currVar) const;

  public:
    /**
     * Constructor.
     */
    COptLogItem(MsgID id, std::string statusDump = "");

    /**
     * Destructor
     */
    virtual ~COptLogItem();

    /**
     * Enter iteration value for this log item.
     * @param unsigned C_INT32 iteration
     * @return COptLogItem & logItem
     */
    COptLogItem & iter(unsigned C_INT32 val);

    /**
     * Enter further values for this log item.
     * @param <T> value
     * @return COptLogItem & logItem
     */
    template <class T>
    COptLogItem & with(T arg)
    {
      std::stringstream s;
      s << arg;
      mMsgVars.push_back(s.str());

      return *this;
    }

    /**
     * Form a plain string of this items data.
     * Does not include status dumps.
     * @return std::string plainMessage
     */
    std::string getPlainMessage() const;

    /**
     * Form a html formatted string of this items data.
     * @return std::string richMessage
     */
    std::string getRichMessage() const;
  };

#endif  // COPASI_COptLogItem
