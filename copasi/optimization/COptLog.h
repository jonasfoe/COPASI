#ifndef COPASI_COptLog
#define COPASI_COptLog

#include <vector>
#include <string>

#include "COptLogItem.h"

class COptLog
  {
  // Attributes
  public:

  protected:

  private:

    /**
     * A vector of all log items
     */
    std::vector< COptLogItem > mLogItems;

  // Operations
  private:

  protected:

  public:
    /**
     * Default constructor.
     */
    COptLog();

    /**
     * Destructor
     */
    virtual ~COptLog();

    /**
     * Enter a log message
     * @param COptLogItem item
     */
    void enterLogItem(COptLogItem item);

    /**
     * Retrieve the method log as plain text.
     * Does not include status dumps.
     * @return std::string plainLog
     */
    std::string getPlainLog() const;

    /**
     * Retrieve the method log as rich text.
     * Uses html headings, divs and tables.
     * @return std::string richLog
     */
    std::string getRichLog() const;
  };

#endif  // COPASI_COptLog
