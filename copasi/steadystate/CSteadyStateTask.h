/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/steadystate/CSteadyStateTask.h,v $
   $Revision: 1.25 $
   $Name:  $
   $Author: shoops $ 
   $Date: 2005/09/15 18:45:25 $
   End CVS Header */

/**
 * CSteadyStateTask class.
 *
 * This class implements a steady state task which is comprised of a
 * of a problem and a method. Additionally calls to the reporting 
 * methods are done when initialized.
 *  
 * Created for Copasi by Stefan Hoops 2002
 */

#ifndef COPASI_CSteadyStateTask
#define COPASI_CSteadyStateTask

#include <iostream>

#include "utilities/CCopasiTask.h"
#include "utilities/CMatrix.h"
#include "utilities/CReadConfig.h"
#include "CSteadyStateMethod.h"
#include "CEigen.h"

class CSteadyStateProblem;
class CState;
class CReportDefinitionVector;

class CSteadyStateTask : public CCopasiTask
  {
    //Attributes
  private:
    /**
     * A pointer to the found steady state.
     */
    CState * mpSteadyState;

    /**
     * A pointer to the found steady state.
     */
    CStateX * mpSteadyStateX;

    /**
     * The jacobian of the steady state.
     */
    CMatrix< C_FLOAT64 > mJacobian;

    /**
     * The jacobian of the steady state.
     */
    CMatrix< C_FLOAT64 > mJacobianX;

    /**
     * Whether the model is actually reducable and calculating 
     * stability of the reduced steady states makes sense
     */
    bool mCalculateReducedSystem;

    /**
     * The Eigenvalues of the Jacobian of the system
     */
    CEigen mEigenValues;

    /**
     * The Eigenvalues of the Jacobian the reduced system
     */
    CEigen mEigenValuesX;

    /**
     * The result of the steady state analysis.
     */
    CSteadyStateMethod::ReturnCode mResult;

    //Operations
  public:

    /**
     * Default constructor
     * @param const CCopasiContainer * pParent (default: NULL)
     */
    CSteadyStateTask(const CCopasiContainer * pParent = & RootContainer);

    /**
     * Copy constructor
     * @param const CSteadyStateTask & src
     * @param const CCopasiContainer * pParent (default: NULL)
     */
    CSteadyStateTask(const CSteadyStateTask & src,
                     const CCopasiContainer * pParent = NULL);

    /**
     * Destructor
     */
    virtual ~CSteadyStateTask();

    /**
     * Initialize the task. If an ostream is given this ostream is used
     * instead of the target specified in the report. This allows nested 
     * tasks to share the same output device.
     * @param const OutputFlag & of
     * @param std::ostream * pOstream (default: NULL)
     * @return bool success
     */
    virtual bool initialize(const OutputFlag & of, std::ostream * pOstream);

    /**
     * Process the task with or without initializing to the initial state.
     * @param const bool & useInitialValues
     * @return bool success
     */
    virtual bool process(const bool & useInitialValues);

    /**
     * This is the output method for any object. The default implementation
     * provided with CCopasiObject uses the ostream operator<< of the object
     * to print the object.To overide this default behaviour one needs to
     * reimplement the virtual print function.
     * @param std::ostream * ostream
     */
    virtual void print(std::ostream * ostream) const;

    /**
     * Loads parameters for this solver with data coming from a
     * CReadConfig object. (CReadConfig object reads an input stream)
     * @param configbuffer reference to a CReadConfig object.
     */
    void load(CReadConfig & configBuffer);

    /**
     * Retrieves a pointer to steady state.
     * @return CState * pSteadyState
     */ 
    //CState * getState();
    const CState * getState() const;

    /**
     * Retrieves a the jacobian of the steady state.
     * @return const CMatrix< C_FLOAT64 > jacobian
     */
    const CMatrix< C_FLOAT64 > & getJacobian() const;

    /**
     * Retrieves a the jacobian of the steady state.
     * @return const CMatrix< C_FLOAT64 > jacobian
     */
    const CMatrix< C_FLOAT64 > & getJacobianReduced() const;

    /**
     * Retrieves a the eigen values of the steady state.
     * @return const CEigen & eigenValues
     */
    const CEigen & getEigenValues() const;

    /**
     * Retrieves a the eigen values of the steady state.
     * @return const CEigen & eigenValues
     */
    const CEigen & getEigenValuesReduced() const;

    const CSteadyStateMethod::ReturnCode & getResult() const {return mResult;}

    // Friend functions
    friend std::ostream &operator<<(std::ostream &os,
                                    const CSteadyStateTask &A);

  private:
    /**
     * cleanup()
     */
    void cleanup();
  };

#endif // COPASI_CSteadyStateTask
