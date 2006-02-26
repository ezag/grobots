// GBSideView.cpp
// Grobots (c) 2002-2004 Devon and Warren Schudy
// Distributed under the GNU General Public License.

#include "GBScoresView.h"
#include "GBSide.h"
#include "GBWorld.h"
#include "GBRobotType.h"
#include "GBStringUtilities.h"

const short kGraphHeight = 100;
const short kInfoBoxHeight = 184;
const short kTotalsHeight = 56;

void GBScoresView::DrawIncome(const GBIncomeStatistics & income,
		short left, short right, short top) {
	long total = income.Total();
	if ( ! total ) return;
	DrawStringPair("Solar:", ToPercentString(income.Autotrophy(), total),
		left, right, top + 10, 9, GBColor::darkGreen);
	DrawStringPair("Manna:", ToPercentString(income.Theotrophy(), total),
		left, right, top + 20, 9, GBColor::darkGreen);
	DrawStringPair("Enemies:", ToPercentString(income.Heterotrophy(), total),
		left, right, top + 30, 9, GBColor::purple);
	DrawStringPair("Stolen:", ToPercentString(income.Kleptotrophy(), total),
		left, right, top + 40, 9, GBColor(0.4f, 0.6f, 0));
	DrawStringPair("Cannibal:", ToPercentString(income.Cannibalism(), total),
		left, right, top + 50, 9, GBColor::darkRed);
}

void GBScoresView::DrawExpenditures(const GBExpenditureStatistics & spent,
		short left, short right, short top) {
	long total = spent.Total();
	if ( ! total ) return;
	DrawStringPair("Growth:", ToPercentString(spent.Construction(), total),
		left, right, top + 10, 9, GBColor::darkGreen);
	DrawStringPair("Engine:", ToPercentString(spent.Engine(), total),
		left, right, top + 20, 9, GBColor::black);
	DrawStringPair("Sensors:", ToPercentString(spent.Sensors(), total),
		left, right, top + 30, 9, GBColor::blue);
	DrawStringPair("Weapons:", ToPercentString(spent.Weapons(), total),
		left, right, top + 40, 9, GBColor::purple);
	DrawStringPair("Force:", ToPercentString(spent.ForceField(), total),
		left, right, top + 50, 9, GBColor::blue);
	DrawStringPair("Shield:", ToPercentString(spent.Shield(), total),
		left, right, top + 60, 9, GBColor::blue);
	DrawStringPair("Repairs:", ToPercentString(spent.Repairs(), total),
		left, right, top + 70, 9, GBColor::black);
	DrawStringPair("Misc:", ToPercentString(spent.Misc(), total),
		left, right, top + 80, 9, GBColor::black);
	DrawStringPair("Stolen:", ToPercentString(spent.Stolen(), total),
		left, right, top + 90, 9, GBColor(0.4f, 0.6f, 0));
	DrawStringPair("Overflow:", ToPercentString(spent.Wasted(), total),
		left, right, top + 100, 9, GBColor::darkRed);
}

void GBScoresView::DrawDeaths(const GBScores & scores,
		short left, short right, short top) {
	DrawStringPair("Kills:", ToPercentString(scores.KilledFraction()),
		left, right, top + 10, 9, GBColor::purple);
	if ( scores.Survived() )
		DrawStringPair("Relative:", ToPercentString(scores.KillRate(), 0),
			left, right, top + 20, 9, GBColor::black);
	DrawStringLongPair("Kills:", scores.Killed(), left, right, top + 30, 9, GBColor::purple);
	DrawStringLongPair("Dead:", scores.Dead(), left, right, top + 40, 9, GBColor::black);
	DrawStringLongPair("Suicide:", scores.Suicide(), left, right, top + 50, 9, GBColor::darkRed);
}

void GBScoresView::DrawGraph(const GBRect & box, long scale,
							 const std::vector<long> & hist, const GBColor & color) {
	int n = hist.size() - 1;
//draw lines
	for ( int i = 0; i < n; ++ i )
		DrawLine(box.left + box.Width() * i / n,
			box.bottom - hist[i] * box.Height() / scale,
			box.left + box.Width() * (i + 1) / n,
			box.bottom - hist[i + 1] * box.Height() / scale,
			color);
}

void GBScoresView::DrawGraphs(const GBRect & box) {
	if ( ! world.Sides() ) return;
	DrawBox(box);
	GBRect graph = box;
	graph.Shrink(1);
	const GBSide * side = world.SelectedSide();
	long scale = 1;
	if ( side ) {
		const std::vector<long> & hist = side->Scores().BiomassHistory();
		for ( int i = 0; i < hist.size(); ++i )
			if (hist[i] > scale)
				scale = hist[i];
	//average biomass, if available
		int rounds = side->TournamentScores().Rounds();
		if ( rounds ) {
			std::vector<long> avg = side->TournamentScores().BiomassHistory();
			avg.resize(hist.size(), 0);
			for ( int i = 0; i < avg.size(); ++i ) {
				avg[i] /= rounds;
				if (avg[i] > scale)
					scale = avg[i];
			}
			DrawGraph(graph, scale, avg,
				rounds > 20 ? GBColor::black : GBColor::darkGray);
		}
		DrawGraph(graph, scale, hist, side->Color().ContrastingTextColor());
	} else {
		const GBSide * s;
		for ( s = world.Sides(); s; s = s->next ) {
			if ( ! s->Scores().Seeded() ) continue;
			const std::vector<long> & hist = s->Scores().BiomassHistory();
			for ( int i = 0; i < hist.size(); ++i )
				if (hist[i] > scale)
					scale = hist[i];
		}
		for ( s = world.Sides(); s; s = s->next )
			DrawGraph(graph, scale, s->Scores().BiomassHistory(),
				s->Color().ContrastingTextColor());
	}
	DrawStringLeft(ToString(scale), box.left + 4, box.top + 13, 10, GBColor::darkGray);
}

void GBScoresView::Draw() {
	const GBSide * side = world.SelectedSide();
	DrawBackground();
//graph
	GBRect box(kEdgeSpace, kEdgeSpace, Width() - kEdgeSpace, kGraphHeight + kEdgeSpace);
	DrawGraphs(box);
//round statistics
	box.right = (Width() - kEdgeSpace) / 2;
	box.top = box.bottom + kEdgeSpace;
	box.bottom = box.top + kInfoBoxHeight;
	DrawBox(box);
	DrawStringLeft(side ? side->Name() : std::string("Overall statistics"),
		box.left + 3, box.top + 13, 12, GBColor::black);
// draw stats...
	short c1 = box.left + 3;
	short c2 = (box.left + box.right) / 2 + 2;
	const GBScores & scores = side ? side->Scores() : world.RoundScores();
// basics
	DrawStringLongPair("Biomass:", scores.Biomass(), c1, c2 - 4, box.top + 25, 9, GBColor::darkGreen);
	DrawStringLongPair("Population:", scores.Population(), c1, c2 - 4, box.top + 35, 9, GBColor::blue);
	DrawStringLongPair("Ever:", scores.PopulationEver(), c1 + 10, c2 - 4, box.top + 45, 9, GBColor::blue);
	if ( side ) {
		if ( side->Scores().Sterile() && side->Scores().SterileTime() != side->Scores().ExtinctTime() )
			DrawStringLongPair("Sterile:", side->Scores().SterileTime(),
				c1, c2 - 4, box.top + 60, 9, GBColor::purple);
		if ( ! side->Scores().Population() )
			DrawStringLongPair("Extinct:", side->Scores().ExtinctTime(),
				c1, c2 - 4, box.top + 70, 9, GBColor::red);
	}
// income
	DrawStringLongPair("Income:", scores.Income().Total(), c1, c2 - 4, box.top + 95, 9, GBColor::black);
	DrawIncome(scores.Income(), c1, c2 - 4, box.top + 95);
	DrawStringLongPair("Seed:", scores.Seeded(), c1, c2 - 4, box.top + 155, 9, GBColor::black);
	if ( scores.Efficiency() > 0 )
		DrawStringPair("Efficiency:", ToPercentString(scores.Efficiency(), 0),
			c1, c2 - 4, box.top + 170, 9, GBColor::black);
	if ( scores.Doubletime(world.CurrentFrame()) )
		DrawStringLongPair("Double:", scores.Doubletime(world.CurrentFrame()),
			c1, c2 - 4, box.top + 180, 9, GBColor::black);
// expenditures
	DrawStringLongPair("Spent:", scores.Expenditure().Total(), c2, box.right - 3, box.top + 25, 9, GBColor::black);
	DrawExpenditures(scores.Expenditure(), c2, box.right - 3, box.top + 25);
// death
	DrawDeaths(scores, c2, box.right - 3, box.top + 130);
// tournament stats:
	box.left = box.right + kEdgeSpace;
	box.right = Width() - kEdgeSpace;
	short c3 = box.left + 3;
	short c4 = (box.left + box.right) / 2 + 2;
	DrawBox(box);
	const GBScores & tscores = side ? side->TournamentScores() : world.TournamentScores();
	DrawStringLongPair("Rounds:", tscores.Rounds(), c3, c4 - 4, box.top + 10, 9, GBColor::black);
	if (tscores.Rounds()) {
		DrawStringPair("Biomass:", ToPercentString(tscores.BiomassFraction()),
			c3, c4 - 4, box.top + 25, 9, GBColor::darkGreen);
		DrawStringPair("Early:", ToPercentString(tscores.EarlyBiomassFraction()),
			c3 + 10, c4 - 4, box.top + 35, 9, GBColor::darkGreen);
		DrawStringPair("Survival:", ToPercentString(tscores.Survival()),
			c3, c4 - 4, box.top + 50, 9, GBColor::black);
		DrawStringPair("Early death:", ToPercentString(tscores.EarlyDeathRate()),
			c3, c4 - 4, box.top + 60, 9, GBColor::black);
		DrawStringPair("Late death:", ToPercentString(tscores.LateDeathRate()),
			c3, c4 - 4, box.top + 70, 9, GBColor::black);
	// income
		DrawStringLongPair("Avg income:", tscores.Income().Total() / tscores.Rounds(),
			c3, c4 - 4, box.top + 95, 9, GBColor::darkGreen);
		DrawIncome(tscores.Income(), c3, c4 - 4, box.top + 95);
		DrawStringLongPair("Avg seed:", tscores.Seeded(),
			c3, c4 - 4, box.top + 155, 9, GBColor::black);
		if ( tscores.Efficiency() > 0 )
			DrawStringPair("Efficiency:", ToPercentString(tscores.Efficiency(), 0),
				c3, c4 - 4, box.top + 170, 9, GBColor::black);
	// expenditures
		DrawStringLongPair("Avg spent:", tscores.Expenditure().Total() / tscores.Rounds(),
			c4, box.right - 4, box.top + 25, 9, GBColor::black);
		DrawExpenditures(tscores.Expenditure(), c4, box.right - 4, box.top + 25);
	// death
		DrawDeaths(tscores, c4, box.right - 4, box.top + 130);
	}
// simulation total values
	box.top = box.bottom + kEdgeSpace;
	box.bottom = box.top + kTotalsHeight;
	DrawBox(box);
	DrawStringLeft("Total values:", box.left + 3, box.top + 13, 10);
	DrawStringLongPair("Manna:", world.MannaValue(),
		box.left + 3, box.right - 3, box.top + 23, 10, GBColor::darkGreen);
	DrawStringLongPair("Corpses:", world.CorpseValue(),
		box.left + 3, box.right - 3, box.top + 33, 10, GBColor::red);
	DrawStringLongPair("Robots:", world.RobotValue(),
		box.left + 3, box.right - 3, box.top + 43, 10, GBColor::blue);
// object counts
	box.left = kEdgeSpace;
	box.right = (Width() - kEdgeSpace) / 2;
	DrawBox(box);
	DrawStringLongPair("Robots:", world.CountObjects(ocRobot),
		box.left + 3, box.right - 3, box.top + 13, 10);
	DrawStringLongPair("Foods:", world.CountObjects(ocFood),
		box.left + 3, box.right - 3, box.top + 23, 10);
	DrawStringLongPair("Shots:", world.CountObjects(ocShot) + world.CountObjects(ocArea),
		box.left + 3, box.right - 3, box.top + 33, 10);
	DrawStringLongPair("Sensors:", world.CountObjects(ocSensorShot),
		box.left + 3, box.right - 3, box.top + 43, 10);
	DrawStringLongPair("Decorations:", world.CountObjects(ocDecoration),
		box.left + 3, box.right - 3, box.top + 53, 10);
// record
	lastDrawnWorld = world.ChangeCount();
	sideID = world.SelectedSideID();
}

GBScoresView::GBScoresView(GBWorld & w)
	: world(w),
	lastDrawnWorld(-1), sideID(-1)
{}

GBMilliseconds GBScoresView::RedrawInterval() const {
	return 2000;
}

bool GBScoresView::InstantChanges() const {
	return sideID != world.SelectedSideID();
}

bool GBScoresView::DelayedChanges() const {
	return lastDrawnWorld != world.ChangeCount();
}

short GBScoresView::PreferredWidth() const {
	return 400;
}

const string GBScoresView::Name() const {
	return "Statistics";
}

short GBScoresView::PreferredHeight() const {
	return kGraphHeight + kInfoBoxHeight + kTotalsHeight + kEdgeSpace * 4;
}

