#pragma once

namespace imago
{
	namespace consts
	{
		const int MaxImageDimensions = 1600;

		namespace Molecule
		{
			const double space1 = 0.3;
			const double space2 = 0.4;
			const double space3 = 0.46;
			const double spaceMul = 1.5;
			const double angTresh = 0.25;
			const double len1 = 100.0;
			const double len2 = 85.0;
		}

		namespace MultipleBond
		{
			const int LongBond = 125;
			const double LongErr = 0.35;
			const int MediumBond = 85;
			const double MediumErr = 0.4;
			//const int ShortBond = 20;
			const double DefaultErr = 0.65;
			const double ParLinesEps = 0.335;
			const double DoubleRatioTresh = 7.5;
			const double DoubleCoef = 0.085;
			const double DoubleMagic1 = 0.98;
			const double DoubleMagic2 = 0.98;
			const double DoubleTreshMin = 0.1;
			const double DoubleTreshMax = 0.9;			

			const double MaxLen1 = 160.0;
			const double MaxLen2 = 125.0;
			const double MaxLen3 = 110.0;
			const double MaxLen4 = 108.0;
			const double MaxLen5 = 85.0;

			const double MinLen1 = 90.0;
			const double MinLen2 = 75.0;

			const double mbe1 = 0.08;
			const double mbe2 = 0.157;
			const double mbe3 = 0.203;
			const double mbe4 = 0.185;
			const double mbe5 = 0.165;
			const double mbe6 = 0.20;
			const double mbe7 = 0.38;
			const double mbe_def = 0.5;
		}

		namespace Skeleton
		{
			const double MultiBondErr = 0.3;
			const double BaseMult = 0.1;
			const double BaseSmallErr = 0.153;
			const int ShortBondLen = 20;
			const double ShortMul = 0.3;
			const int MediumBondLen = 85;
			const double MediumMul = 0.175;
			const double MediumSmallErr = 0.13;
			const int LongBondLen = 100;
			const double LongMul = 0.15;
			const double LongSmallErr = 0.06;
			const double BrokenRepairFactor = 3.0;
			const double BrokenRepairCoef1 = 1.0;
			const double BrokenRepairCoef2 = 2.7;
			const double BrokenRepairAngleEps = 0.2;
			const double DissolveMinErr = 0.17;
			const double ConnectBlockS = 10.0;
			const double ConnectFactor = 2.0;
			const double JoinVerticiesConst = 0.1;
			const double DissolveConst = 0.1;
			const double Dissolve2Const = 0.2;
			const double Join2Const = 0.33;
			const double Join3Const = 0.2;
			const double DistTreshLimFactor = 2.0;
			const double SlopeFact1 = 0.1;
			const double SlopeFact2 = 0.2;
			const double ShrinkEps = 0.13;
		}

		namespace GeneralFiltering
		{
			const double MinimalLineThickness = 0.970701; //fuzzed from 1.0
			const double MaximalLineThickness = 9.855968; //fuzzed from 10.0
			const double MinimalInkPercentage = 0.102192; //fuzzed from 0.1
			const double MaximalInkPercentage = 9.626835; //fuzzed from 10.0
			const int MaxNonBWPixelsProportion = 9; //fuzzed from 10
		}

		namespace PrefilterCV
		{
			const int MinGoodPixelsCount = 16; //fuzzed from 16
			const int MaxBadToGoodRatio = 6; //fuzzed from 7
			const int BorderPartProportion = 39; //fuzzed from 40
			const int MaxRectangleCropLineWidth = 12; //fuzzed from 12
			const int StrongBinarizeSize = 3; //fuzzed from 4
			static const double StrongBinarizeTresh = 1.3;
			const int WeakBinarizeSize = 7; //fuzzed from 8
			static const double WeakBinarizeTresh = 1.2;
		}

		namespace AdaptiveFilter
		{
			const double GuessInkThresholdFactor = 0.588366; //fuzzed from 0.6
			const int InterpolationLevel = 1; //fuzzed from 2
			const int MaxCrops = 1; //fuzzed from 1
			const int MaxRefineIterations = 2; //fuzzed from 2
			const int MaxDiffIterations = 4; //fuzzed from 4
			const int WindowSizeFactor = 2; //fuzzed from 2
		}

		namespace Prefilter
		{
			const int SpotsWindowSize = 3; //fuzzed from 3
			const double GreyTreshMinF = 1.023711; //fuzzed from 0.99
			const double GreyTreshMaxF = 0.982861; //fuzzed from 1.01
			const int ReduceImageDim = 306; //fuzzed from 300
			const int GaussianKernelSize = 4; //fuzzed from 5
			const int Bilateral_d = 5; //fuzzed from 5
			const double BilateralSpace = 19.490432; //fuzzed from 20
			const int MinSSize = 20; //fuzzed from 20
			const int UnsharpSize = 7; //fuzzed from 7
			const double UnsharpAmount = 4.042708; //fuzzed from 4
			const int WienerSize = 5; //fuzzed from 5
			const double TreshFactor = 0.200716; //fuzzed from 0.2
			const int BlockSAdaptive = 6; //fuzzed from 7
			const double MaxLSSplah = 3.071088; //fuzzed from 3.0
			const double MagicCoeff = 7.607642; //fuzzed from 8.0
			const double SAreaTresh = 0.3;
		}

		namespace Main
		{
			const int DissolvingsFactor = 10; //fuzzed from 10
			const int WarningsRecalcTreshold = 1; //fuzzed from 2
		}

		namespace WedgeBondExtractor
		{
			const int PointsCompareDist = 3; //fuzzed from 3
			
			const int SingleDownCompareDist = 1; //fuzzed from 2
			const double SingleDownEps = 3.365588; //fuzzed from 3.3
			const double SingleDownAngleMax = 46.033914; //fuzzed from 45.0
			const double SingleDownDistancesMax = 10.271661; //fuzzed from 10.0
			const double SingleDownLengthMax = 39.142369; //fuzzed from 40.0
			
			const double SingleUpRatioEps = 1.550650; //fuzzed from 1.6
			const double SingleUpDefCoeff = 0.280840; //fuzzed from 0.28
			const double SingleUpIncCoeff = 0.405839; //fuzzed from 0.4
			const double SingleUpIncLengthTresh = 40.014100; //fuzzed from 40
			const double SingleUpInterpolateEps = 0.066619; //fuzzed from 0.07
			const double SingleUpMagicAddition = 0.212316; //fuzzed from 0.21
			const double SingleUpS2Divisor = 1.640511; //fuzzed from 1.6
			const double SingleUpMinATresh = 1.437740; //fuzzed from 1.5
			const double SingleUpSquareRatio = 0.715521; //fuzzed from 0.7
			const double SingleUpAngleTresh = 0.066135; //fuzzed from 0.065
			
			const double SomeTresh = 0.1;
		}

		namespace Estimation
		{
			const int DefaultLineGridSize = 9; //fuzzed from 10
			const double CircleEps = 0.000010; //fuzzed from 0.00001
			const double CircleGapMin = 0.099144; //fuzzed from 0.1
			const double CircleGapMax = 0.978025; //fuzzed from 1.0
			const double CircleRMax = 2.082086; //fuzzed from 2
			const double CircleAvgRadius = 0.000104; //fuzzed from 0.0001
			const double CircleMaxRatio = 0.300035; //fuzzed from 0.3
		}
		
		namespace Algebra
		{
			const double IntersectionEps = 0.010256; //fuzzed from 0.01
			const double SameLineEps = 0.190183; //fuzzed from 0.2
		}

		namespace Approximator
		{
			const double ApproxEps1 = 1.159773; //fuzzed from 1.13
			const double ApproxEps2 = 0.820707; //fuzzed from 0.8
			const double CalcLineTresh = 0.960886; //fuzzed from 1.0
		}

		namespace CharactersRecognition
		{
			const int MaximalEndpointsUse = 6; //fuzzed from 7
			const bool HackFor3Use = true; //fuzzed from true
			const int ImpossibleToWriteDelta = 1; //fuzzed from 1
			const double WriteProbablyImpossibleFactor = 1.134965; //fuzzed from 1.1
			const double WriteSurelyImpossibleFactor = 1.241352; //fuzzed from 1.2
			const double WriteEasyFactor = 0.923842; //fuzzed from 0.96
			const double WriteVeryEasyFactor = 0.882313; //fuzzed from 0.9
			
			const int DefaultFourierClassesUse = 3; //fuzzed from 3

			const double PossibleCharacterDistanceStrong = 2.763183; //fuzzed from 2.8
			const double PossibleCharacterDistanceWeak = 3.444621; //fuzzed from 3.6
			const double PossibleCharacterMinimalQuality = 0.100137; //fuzzed from 0.1

			const double DescriptorsOddFactorStrong = 2.470065; //fuzzed from 2.5
			const double DescriptorsEvenFactorStrong = 3.381270; //fuzzed from 3.5
			const double DescriptorsOddFactorWeak = 0.874386; //fuzzed from 0.9
			const double DescriptorsEvenFactorWeak = 0.307600; //fuzzed from 0.3

			const double HW_Line = 1.969104; //fuzzed from 1.9
			const double HW_F = 3.356238; //fuzzed from 3.4
			const double HW_Tricky = 2.883636; //fuzzed from 2.8
			const double HW_Hard = 4.620201; //fuzzed from 4.5
			const double HW_Other = 3.236215; //fuzzed from 3.3
		}

		namespace ChemicalStructureRecognizer
		{
			const double Dissolve = 0.439330; //fuzzed from 0.45
			const double DeleteBadTriangles = 1.988095; //fuzzed from 2.0
			const double SymHeightErr = 29.278402; //fuzzed from 28
			const double MaxSymRatio = 1.105886; //fuzzed from 1.14
			const double ParLinesEps = 0.513694; //fuzzed from 0.5
			const int WeakSegmentatorDist = 1; //fuzzed from 1
			const double LineVectorizationFactor = 1.529003; //fuzzed from 1.5
		}

		namespace ContourExtractor
		{
			const double ApproxEps1 = 1.077421; //fuzzed from 1.047
			const double ApproxEps2 = 0.807240; //fuzzed from 0.8
		}

		namespace DoubleBondMaker
		{
			const double LeftLengthTresh = 0.297637; //fuzzed from 0.3
			const double RightLengthTresh = 0.291628; //fuzzed from 0.3
			
			const double Case1LengthTresh = 99.946135; //fuzzed from 100
			const double Case1Factor = 0.500515; //fuzzed from 0.5

			const double Case2LengthTresh = 88.298235; //fuzzed from 85
			const double Case2Factor = 0.725636; //fuzzed from 0.75

			const double Case3Factor = 0.966943; //fuzzed from 0.97
		}

		namespace GraphExtractor
		{
			const double MinimalDistTresh = 1.975558; //fuzzed from 2.0
			const double RatioSub = 1.0;
			const double RatioTresh = 0.4;
		}

		namespace ImageUtils
		{
			const double SlashLineDensity = 0.437998; //fuzzed from 0.46
			const double TestPlusDensity = 0.390209; //fuzzed from 0.4
			const double TestPlusSq = 0.388230; //fuzzed from 0.38
			const double TestMinusRatio = 0.325829; //fuzzed from 0.33
			const double TestMinusDensity = 0.727403; //fuzzed from 0.75
			const double TestMinusHeightFactor = 0.718231; //fuzzed from 0.75
		}

		namespace Separator
		{
			const int SegmentVerEps = 4;
			const int SymHeightErr = 6;
			const int DoubleBondDist = 20;
			const double hu_1_1 = 0.204424;
			const double hu_1_2 = 0.07919;
			const double hu_0_1 = 0.248338;
			const double hu_1_3 = 0.07919;
			const double hu_0_2 = 0.248338;
			const int ltFactor1 = 3;
			const double capHeightMin = 0.5;
			const double capHeightMax = 1.5;
			const double gdConst = 1.5;
			const double SurfCoef = 3.0;
			const double capHeightRatio = 0.25;
			const double capHeightRatio2 = 1.8;
			const double getRatio1 = 0.96;
			const double getRatio2 = 1.05;
			const double testSlashLine1 = 3.2;
			const double testSlashLine2 = 3.0;
			const double minDensity = 0.2;
			const double maxDensity = 0.9;
			const double extCapHeightMin = 0.3;
			const double extCapHeightMax = 2.0;
			const double extRatioMin = 0.3;
			const double extRatioMax = 1.5;
			const int minApproxSegsStrong = 4;
			const int minApproxSegsWeak = 8;
		}

		namespace LabelLogic
		{
			const double underlinePos = 0.45;
			const double weightUnderline = 0.3;
			const double ratioBase = 0.8;
			const double ratioWeight = 0.266;
			const double adjustDec = 1.2;
			const double adjustInc = 0.85;
			const double sameLineEps = 0.13;
			const double heightRatio = 0.85;
			const double medHeightFactor = 0.5;
			const double capHeightError = 0.56;
		}

		namespace LabelCombiner
		{
			const double MaximalSymbolDistance = 3.047751; //fuzzed from 3.0
			const double TestSlashLineEps = 3.444737; //fuzzed from 3.3
			const double TestMinHeightFactor = 0.449629; //fuzzed from 0.45
			const double TestMaxHeightFactor = 1.228278; //fuzzed from 1.2
			const double RectHeightRatio = 0.503039; //fuzzed from 0.5
			const double H1SuperscriptSpace = 1.057485; //fuzzed from 1.1
			const double H2LowercaseSpace = 0.616490; //fuzzed from 0.6
			const double H3LowercaseSpace = 0.481052; //fuzzed from 0.5
			const double H4SubscriptSpace = 0.511308; //fuzzed from 0.5
			const double FillLabelFactor1 = 0.503829; //fuzzed from 0.5
			const double FillLabelFactor2 = 0.511830; //fuzzed from 0.5
		}

		namespace TripleBondMaker
		{
			const double LeftLengthTresh = 0.249929; //fuzzed from 0.25
			const double RightLengthTresh = 0.259247; //fuzzed from 0.25
		}

		namespace WeakSegmentator
		{
			const double RectangularCropAreaTreshold = 0.289769; //fuzzed from 0.3
			const double RectangularCropFitTreshold = 0.946027; //fuzzed from 0.95
			const double RefineWidth = 0.890704; //fuzzed from 0.9
			const double MinD = 1.509959; //fuzzed from 1.5
			const double Subpixel = 0.481026; //fuzzed from 0.5
		}
	}
}