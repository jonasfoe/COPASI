package org.COPASI.gui;

import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JLabel;
import javax.swing.JTextField;

import java.awt.Insets;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import org.COPASI.CCopasiDataModel;
import org.COPASI.CCopasiParameter;
import org.COPASI.CCopasiProblem;
import org.COPASI.CCopasiTask;

import java.awt.event.FocusEvent;
import java.awt.event.FocusListener;

public class TrajectoryTaskWidget extends TaskWidget  implements FocusListener{

	/**
	 * 
	 */
	private static final long serialVersionUID = 1950018411659131759L;
	protected TrajectoryProblemWidgetContent mProblemWidgetContent;
	
	public class TrajectoryProblemWidgetContent extends JPanel {
		/**
		 * 
		 */
		private static final long serialVersionUID = -1904794840441682301L;
		protected JTextField mDurationInput;
		protected JTextField mIntervalSizeInput;
		protected JTextField mNumIntervalsInput;
		protected JTextField mStartOutputTimeInput; 
		
		
		public TrajectoryProblemWidgetContent() {
			GridBagLayout layout=new GridBagLayout();
			this.setLayout(layout);
			GridBagConstraints gbc=new GridBagConstraints();
			
			// Festlegen, dass die GUI-Elemente die Gitterfelder in 
	                // waagerechter Richtung ausf�llen:
			gbc.fill=GridBagConstraints.NONE;
			
			// Die Abst�nder der einzelnen GUI-Elemente zu den gedachten 
	                // Gitterlinien festgelegen:
			gbc.insets = new Insets(2,2,2,2);  

			gbc.gridx = 0;  // x-Position im gedachten Gitter
			gbc.gridy = 0;  // y-Position im gedachten Gitter
			gbc.gridheight = 1;  // ein Gitter-Felder hoch
			JLabel label = new JLabel("Duration");
			layout.setConstraints(label, gbc);
			this.add(label);
			gbc.gridx = 0;  // x-Position im gedachten Gitter
			gbc.gridy = 1;  // y-Position im gedachten Gitter
			label = new JLabel("Interval Size");
			layout.setConstraints(label, gbc);
			this.add(label);

			gbc.gridx = 1;  // x-Position im gedachten Gitter
			gbc.gridy = 0;  // y-Position im gedachten Gitter
			gbc.fill=GridBagConstraints.HORIZONTAL;
			gbc.weightx=0.5;
			this.mDurationInput = new JTextField("1",15);
			layout.setConstraints(this.mDurationInput, gbc);
			this.add(this.mDurationInput);
			this.mDurationInput.setInputVerifier(new FloatVerifier());
			gbc.gridx = 1;  // x-Position im gedachten Gitter
			gbc.gridy = 1;  // y-Position im gedachten Gitter
			this.mIntervalSizeInput= new JTextField("0.01",15);
			layout.setConstraints(this.mIntervalSizeInput, gbc);
			this.add(this.mIntervalSizeInput);
			this.mIntervalSizeInput.setInputVerifier(new FloatVerifier());

			gbc.insets = new Insets(2,8,2,2);  
			gbc.gridx = 2;  // x-Position im gedachten Gitter
			gbc.gridy = 0;  // y-Position im gedachten Gitter
			gbc.fill=GridBagConstraints.NONE;
			gbc.weightx=0.0;
			label = new JLabel("Intervals");
			layout.setConstraints(label, gbc);
			this.add(label);
			gbc.gridx = 2;  // x-Position im gedachten Gitter
			gbc.gridy = 1;  // y-Position im gedachten Gitter
			label = new JLabel("Start Output Time");
			layout.setConstraints(label, gbc);
			this.add(label);

			gbc.insets = new Insets(2,2,2,2);  
			gbc.gridx = 3;  // x-Position im gedachten Gitter
			gbc.gridy = 0;  // y-Position im gedachten Gitter
			gbc.fill=GridBagConstraints.HORIZONTAL;
			gbc.weightx=0.5;
			this.mNumIntervalsInput = new JTextField("100",15);
			layout.setConstraints(this.mNumIntervalsInput, gbc);
			this.mNumIntervalsInput.setInputVerifier(new PositiveIntegerVerifier());
			this.add(this.mNumIntervalsInput);
			gbc.gridx = 3;  // x-Position im gedachten Gitter
			gbc.gridy = 1;  // y-Position im gedachten Gitter
			this.mStartOutputTimeInput= new JTextField("0",15);
			layout.setConstraints(this.mStartOutputTimeInput, gbc);
			this.add(this.mStartOutputTimeInput);
			this.mStartOutputTimeInput.setInputVerifier(new FloatVerifier());

			
		}





	}

	public TrajectoryTaskWidget(boolean enableReportButton) 
	{
		super("Time Course",enableReportButton);
		this.mProblemWidgetContent=new TrajectoryProblemWidgetContent();
		
		this.mProblemWidget.add(this.mProblemWidgetContent);
		this.mProblemWidgetContent.mDurationInput.addFocusListener(this);
		this.mProblemWidgetContent.mIntervalSizeInput.addFocusListener(this);
		this.mProblemWidgetContent.mNumIntervalsInput.addFocusListener(this);
		this.mProblemWidgetContent.mStartOutputTimeInput.addFocusListener(this);
		this.resetTask();

	}

	protected void resetTask()
	{
		CCopasiDataModel m=CCopasiDataModel.getGlobal();
		CCopasiTask t=null;
		long size=m.getTaskList().size();
		long i=0;
		while(t==null && i < size)
		{
			if(m.getTask(i).getType()==CCopasiTask.timeCourse)
			{
				t=m.getTask(i);
			}
			i++;
		}
		if(t!=null)
		{
			this.setTask(t);
		}
	}
	
	public void setTask(CCopasiTask task)
	{
		super.setTask(task);
		this.setProblemParameters();
	}
	
	protected void setProblemParameters()
	{
		// set duration, interval size , interval number and output start time in the gui		
		CCopasiProblem problem=this.mTask.getProblem();
		for(int i=0;i<problem.size();i++)
		{
			CCopasiParameter p=problem.getParameter(i);
			String parameterName=p.getObjectName();
			if(parameterName.equals("Duration"))
			{
				this.mProblemWidgetContent.mDurationInput.setText(new Double(p.getDblValue()).toString());
			}
			else if(parameterName.equals("OutputStartTime"))
			{
				this.mProblemWidgetContent.mStartOutputTimeInput.setText(new Double(p.getDblValue()).toString());
			}
			else if(parameterName.equals("StepSize"))
			{
				// step number is calculated from the step size and the duration
				this.mProblemWidgetContent.mIntervalSizeInput.setText(new Double(p.getDblValue()).toString());			
			}
		}
	}

	public void focusGained(FocusEvent e) {
		// do nothin
	}


	public void focusLost(FocusEvent e) {
		// set the value on the variable
		Object s=e.getSource();
		if(s==this.mProblemWidgetContent.mDurationInput)
		{
			// set the value
			if(this.mTask!=null)
			{
				Double d=new Double(((JTextField)s).getText());
				CCopasiParameter p=this.mTask.getProblem().getParameter("Duration");
				if(p!=null)
				{
					p.setDblValue(d.doubleValue());
				}
				// recalculate the interval size
				p=this.mTask.getProblem().getParameter("StepNumber");
				if(p!=null)
				{
					double v=d/p.getUIntValue();
					p=this.mTask.getProblem().getParameter("StepSize");
					p.setDblValue(v);
					this.mProblemWidgetContent.mIntervalSizeInput.setText(new Double(v).toString());
					
				}
			}
		}
		else if(s==this.mProblemWidgetContent.mIntervalSizeInput)
		{
			// set the value and recalculate the number of intervals and set it
			if(this.mTask!=null)
			{
				Double d=new Double(((JTextField)s).getText());
				CCopasiParameter p=this.mTask.getProblem().getParameter("StepSize");
				if(p!=null)
				{
					p.setDblValue(d.doubleValue());
				}
				p=this.mTask.getProblem().getParameter("Duration");
				if(p!=null)
				{
					long v=(long)Math.ceil(Math.abs(p.getDblValue()/d));
					p=this.mTask.getProblem().getParameter("StepNumber");
					p.setUIntValue(v);
					this.mProblemWidgetContent.mIntervalSizeInput.setText(new Long(v).toString());
					
				}
				
			}
		}
		else if(s==this.mProblemWidgetContent.mNumIntervalsInput)
		{
			// set the value and recalculate the interval size and set it
			if(this.mTask!=null)
			{
				Long l=new Long(((JTextField)s).getText());
				CCopasiParameter p=this.mTask.getProblem().getParameter("StepNumber");
				if(p!=null)
				{
					p.setDblValue(l.doubleValue());
				}
				p=this.mTask.getProblem().getParameter("Duration");
				if(p!=null)
				{
					double v=p.getDblValue()/l;
					p=this.mTask.getProblem().getParameter("StepSize");
					p.setDblValue(v);
					this.mProblemWidgetContent.mIntervalSizeInput.setText(new Double(v).toString());
					
				}
				
			}
		}
		else if(s==this.mProblemWidgetContent.mStartOutputTimeInput)
		{
			// set the value
			if(this.mTask!=null)
			{
				Double d=new Double(((JTextField)s).getText());
				CCopasiParameter p=this.mTask.getProblem().getParameter("OutputStartTime");
				if(p!=null)
				{
					p.setDblValue(d.doubleValue());
				}
			}
		}
	}
	
	public void createDefaultReportDefiniton()
	{
		if(this.mTask!=null)
		{
			
		}
	}
	
	
	/**
	 * @param args
	 */
	public static void main(String[] args) {

	       javax.swing.SwingUtilities.invokeLater
	        (
	          new Runnable() 
	          {
	            public void run() 
	            {
	            	JFrame mainDialog=new JFrame();
	            	mainDialog.setTitle("Task Dialog");
	        		mainDialog.getContentPane().add(new TrajectoryTaskWidget(true));
	        	 	mainDialog.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
	        		mainDialog.pack();
	        		mainDialog.setVisible(true);
	            }
	          }
	        );	
	      }

}
