using System;
using System.Collections.Generic;
using System.IO;

namespace CourseProject
{
	class MainClass
	{
		private static int startPoint;
		private static int numOfPC;
		private static double failRate;
		private static double repairRate;
		private static int numOfRepair;

		private static int indexOfDifference;
		private static List<double> rangeOfDifference = new List<double>();
		
		private static string outputName;
		
		public static void Init(string name)
		{
			var confLines = File.ReadAllLines(name);
		
			/* --- Init first configuration --- */
			var confElements = confLines[0].Split(' ');
			numOfPC = int.Parse(confElements[0]);
			startPoint = int.Parse(confElements[1]);
			failRate = Math.Pow(10, int.Parse(confElements[2]));
			repairRate = int.Parse(confElements[3]);
			numOfRepair = int.Parse(confElements[4]);
			/* --- Init first configuration --- */


			/* --- Obtain index of change value --- */
			indexOfDifference = int.Parse(confLines[1]);
			/* --- Obtain index of change value --- */

			/* --- Obtain range of difference --- */
			foreach (var rangeElement in confLines[2].Split(' '))
			{ rangeOfDifference.Add(double.Parse(rangeElement)); }
			/* --- Obtain range of difference --- */
		}

		public static void Main (string[] args)
		{
			/* --- некорректные входные данные --- */
			if(args.Length < 2)
			{
				Console.WriteLine ("Входные параметры не заданы");
				Environment.Exit(-1);
			}
			/* --- некорректные входные данные --- */

			Init(string.Format("i_{0}.in",args[0]));
			var dataArray = new double[24 / 2, rangeOfDifference.Count];

			for (int p = 0; p < rangeOfDifference.Count; p++)
			{
				/* --- change value of parameter --- */
				switch (indexOfDifference)
				{
					case 0:
						throw new Exception("Number of machine is constant");
						break;

					case 1:
						startPoint = (int)rangeOfDifference[p];
						break;

					case 2:
						failRate = Math.Pow(10, (int)rangeOfDifference[p]);
						break;

					case 3:
						repairRate = (int)rangeOfDifference[p];
						break;

					case 4:
						numOfRepair = (int)rangeOfDifference[p];
						break;
				}
				/* --- change value of parameter --- */

				/* --- perform calculations --- */
				for (int t = 0; t < 24; t += 2)
				{
					dataArray[t/2, p] = int.Parse(args[1]) == 0 ? Cacl_Survivability(t) : Cacl_Employment(t);
					Console.Write(".");
				}
				/* --- perform calculations --- */
				Console.Write("\n");
			}

			SaveToFile(string.Format("o_{0}.{1}.data",args[0], args[1]), dataArray);
		}

		public static double Cacl_Survivability( double timeVar)
		{
			double retVal;
			var curN = numOfPC;

			retVal = (repairRate/(failRate + repairRate)) +
					 ((startPoint*failRate - (curN - startPoint)*repairRate)/(curN*(failRate + repairRate)))*
					 Math.Exp(-(failRate + repairRate)*timeVar);

			return retVal;
		}

		public static double Cacl_Employment(double timeVar)
		{
			double retVal;
			var curN = numOfPC;

			retVal = (curN*failRate / (numOfRepair * (failRate + repairRate))) -
					 ((startPoint * failRate - (curN - startPoint) * repairRate) / (numOfRepair * (failRate + repairRate))) *
					 Math.Exp(-(failRate + repairRate) * timeVar);

			return retVal;
		}

		public static void SaveToFile(string name, double[,]  data)
		{
			Console.WriteLine("Start writing to disk...");
			using (var sw = new StreamWriter(string.Concat(name)))
			{
				for (int t = 0; t < 24; t+=2)
				{
					var tmpStr = string.Format("{0} ", t);
					for (int j = 0; j < data.GetLength(1); j++)
					{ tmpStr += string.Format("{0:r2} ", data[t/2,j]); }
					
					sw.WriteLine(tmpStr.Replace(",", "."));
				}
			}
			
			Console.WriteLine("Wrote. Press any key bla bla bla ");
		}

		public static double Get_N()
		{ return repairRate / (failRate + repairRate); }

		public static double Get_M()
		{ return (Get_N() * failRate) / Math.Ceiling(numOfRepair * (failRate + repairRate)); }
	}
}