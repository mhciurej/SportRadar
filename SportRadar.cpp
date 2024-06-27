//
//  main.cpp
//  SportRadar
//
//  Created by Michał Ciurej on 24/06/2024.
//

#include <iostream>
#include <set>
#include <unordered_set>
#include <ranges>

#include "gtest/gtest.h"

struct match
{
    std::string home;
    std::string away;
    int home_goals;
    int away_goals;
    match(const std::string& home, const std::string& away, int home_goals, int away_goals) : home(home), away(away), home_goals(home_goals), away_goals(away_goals) {}
    match(const std::string& home, const std::string& away) : match(home, away, 0, 0) {}
    bool operator==(const match& to_compare) const
    {
        return home == to_compare.home and away == to_compare.away;
    }
};

template <>
struct std::hash<match>
{
    std::size_t operator()(const match& s) const noexcept
    {
        return std::hash<std::string>()(s.home) & std::hash<std::string>()(s.away);
    }
};

struct match_node
{
    match m;
    size_t order;
    match_node(const match& m, size_t order) : m(m), order(order) {}
    bool operator<(const match_node& to_compare) const
    {
        if(m.home_goals + m.away_goals != to_compare.m.home_goals + to_compare.m.away_goals)
        {
            return m.home_goals + m.away_goals > to_compare.m.home_goals + to_compare.m.away_goals;
        }
        else
        {
            return order < to_compare.order;
        }
    }
};

class scoreboard
{
    size_t counter;
    std::set<match_node> board;
    std::unordered_map<match, size_t> match_to_order;
public:
    scoreboard() : counter(0) {}
    void start(const std::string& home, const std::string& away)
    {
        match new_match(home, away);
        board.insert(match_node(new_match, counter));
        match_to_order[new_match] = counter;
        counter++;
    }
    void update(const match& new_match)
    {
        auto elem = match_to_order.find(new_match);
        auto [old_match, order] = *elem;
        board.erase(match_node(old_match, order));
        board.insert(match_node(new_match, order));
        match_to_order.erase(elem);
        match_to_order[new_match] = order;
    }
    void finish(const std::string& home, const std::string& away)
    {
        match to_remove(home, away);
        if(match_to_order.contains(to_remove))
        {
            auto elem = match_to_order.find(to_remove);
            auto& [old_match, order] = *elem;
            board.erase(match_node(old_match, order));
            match_to_order.erase(elem);
        }
    }
    std::vector<match> summary() const
    {
        auto matches = board | std::views::transform([](const match_node& mn){ return mn.m; });
        return std::vector<match>(matches.begin(), matches.end());
    }
};




int main(int argc, const char * argv[]) {
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}

TEST(SportRadar, Initial)
{
    scoreboard s;
    EXPECT_EQ(s.summary(), std::vector<match>());
}

TEST(SportRadar, JustCreate)
{
    scoreboard s;
    s.start("T1", "T2");
}

TEST(SportRadar, SummaryTest)
{
    scoreboard s0;
    EXPECT_EQ(s0.summary().size(), 0);
    
    scoreboard s1;
    s1.start("T1", "T2");
    EXPECT_EQ(s1.summary().size(), 1);
    
    scoreboard s2;
    s2.start("T1", "T2");
    s2.start("T3", "T4");
    EXPECT_EQ(s2.summary().size(), 2);
}

TEST(SportRadar, CreateAndSeeEmpty)
{
    scoreboard s;
    s.start("T1", "T2");
    match result = s.summary().front();
    EXPECT_EQ(result.home, "T1");
    EXPECT_EQ(result.away, "T2");
    EXPECT_EQ(result.home_goals, 0);
    EXPECT_EQ(result.away_goals, 0);
}

TEST(SportRadar, TwoTeams)
{
    scoreboard s;
    s.start("T1", "T2");
    s.start("T3", "T4");
    match result = s.summary()[0];
    EXPECT_EQ(result.home, "T1");
    EXPECT_EQ(result.away, "T2");
    EXPECT_EQ(result.home_goals, 0);
    EXPECT_EQ(result.away_goals, 0);
    match result1 = s.summary()[1];
    EXPECT_EQ(result1.home, "T3");
    EXPECT_EQ(result1.away, "T4");
    EXPECT_EQ(result1.home_goals, 0);
    EXPECT_EQ(result1.away_goals, 0);
}

TEST(SportRadar, SimpleUpdate)
{
    scoreboard s;
    s.start("T1", "T2");
    s.update({ "T1", "T2", 1, 0 });
    match result = s.summary().front();
    
    EXPECT_EQ(result.home, "T1");
    EXPECT_EQ(result.away, "T2");
    EXPECT_EQ(result.home_goals, 1);
    EXPECT_EQ(result.away_goals, 0);
}

TEST(SportRadar, UpdateWithoutImpactOnOrder)
{
    scoreboard s;
    s.start("T1", "T2");
    s.start("T3", "T4");
    s.update({ "T1", "T2", 1, 0 });
    match result = s.summary().front();
    
    EXPECT_EQ(result.home, "T1");
    EXPECT_EQ(result.away, "T2");
    EXPECT_EQ(result.home_goals, 1);
    EXPECT_EQ(result.away_goals, 0);
}

TEST(SportRadar, UpdateWithImpactOnOrder)
{
    scoreboard s;
    s.start("T1", "T2");
    s.start("T3", "T4");
    s.update({ "T3", "T4", 1, 0 });
    match result = s.summary().front();
    
    EXPECT_EQ(result.home, "T3");
    EXPECT_EQ(result.away, "T4");
    EXPECT_EQ(result.home_goals, 1);
    EXPECT_EQ(result.away_goals, 0);
}

TEST(SportRadar, SumOfPointsDeterminesOrder)
{
    scoreboard s;
    s.start("T1", "T2");
    s.start("T3", "T4");
    s.start("T5", "T6");
    s.start("T7", "T8");
    s.start("T9", "T10");
    s.update({ "T1", "T2", 1, 0 });
    s.update({ "T3", "T4", 4, 0 });
    s.update({ "T5", "T6", 2, 0 });
    s.update({ "T7", "T8", 3, 0 });
    auto result = s.summary();
    for (int i = 4; auto r : result) {
        EXPECT_EQ(r.home_goals, i);
        i--;
    }
}

TEST(SportRadar, OrderChangesMultipleTimes)
{
    scoreboard s;
    s.start("T1", "T2");
    s.start("T3", "T4");
    s.start("T5", "T6");
    s.start("T7", "T8");
    s.update({ "T5", "T6", 1, 0 });
    s.update({ "T3", "T4", 1, 0 });

    match result0 = s.summary()[0];
    EXPECT_EQ(result0.home, "T3");
    EXPECT_EQ(result0.away, "T4");
    EXPECT_EQ(result0.home_goals, 1);
    EXPECT_EQ(result0.away_goals, 0);

    match result1 = s.summary()[1];
    EXPECT_EQ(result1.home, "T5");
    EXPECT_EQ(result1.away, "T6");
    EXPECT_EQ(result1.home_goals, 1);
    EXPECT_EQ(result1.away_goals, 0);
    
    match result2 = s.summary()[2];
    EXPECT_EQ(result2.home, "T1");
    EXPECT_EQ(result2.away, "T2");
    EXPECT_EQ(result2.home_goals, 0);
    EXPECT_EQ(result2.away_goals, 0);
    
    match result3 = s.summary()[3];
    EXPECT_EQ(result3.home, "T7");
    EXPECT_EQ(result3.away, "T8");
    EXPECT_EQ(result3.home_goals, 0);
    EXPECT_EQ(result3.away_goals, 0);
    
    s.update({ "T5", "T6", 1, 1 });
    
    match result4 = s.summary()[0];
    EXPECT_EQ(result4.home, "T5");
    EXPECT_EQ(result4.away, "T6");
    EXPECT_EQ(result4.home_goals, 1);
    EXPECT_EQ(result4.away_goals, 1);

    match result5 = s.summary()[1];
    EXPECT_EQ(result5.home, "T3");
    EXPECT_EQ(result5.away, "T4");
    EXPECT_EQ(result5.home_goals, 1);
    EXPECT_EQ(result5.away_goals, 0);
    
    match result6 = s.summary()[2];
    EXPECT_EQ(result6.home, "T1");
    EXPECT_EQ(result6.away, "T2");
    EXPECT_EQ(result6.home_goals, 0);
    EXPECT_EQ(result6.away_goals, 0);
    
    match result7 = s.summary()[3];
    EXPECT_EQ(result7.home, "T7");
    EXPECT_EQ(result7.away, "T8");
    EXPECT_EQ(result7.home_goals, 0);
    EXPECT_EQ(result7.away_goals, 0);
}

TEST(SportRadar, OrderByInitialOrderWhenPointsAreEqual)
{
    scoreboard s;
    s.start("T1", "T2");
    s.start("T3", "T4");
    s.start("T5", "T6");
    s.start("T7", "T8");
    s.update({ "T5", "T6", 1, 0 });
    s.update({ "T7", "T8", 1, 0 });
    s.update({ "T3", "T4", 1, 0 });
    
    EXPECT_EQ(s.summary().front(), match("T3", "T4", 1, 0));
    
    s.update({ "T7", "T8", 1, 1 });
    EXPECT_EQ(s.summary().front(), match("T7", "T8", 1, 1));
}

TEST(SportRadar, Finishing)
{
    scoreboard s;
    s.start("T1", "T2");
    s.finish("T1", "T2");
    EXPECT_EQ(s.summary(), std::vector<match>());
}

TEST(SportRadar, FinishingDoesNotImpactOrder)
{
    scoreboard s;
    s.start("T1", "T2");
    s.start("T3", "T4");
    s.start("T5", "T6");
    s.start("T7", "T8");
    s.finish("T3", "T4");
    
    match result0 = s.summary()[0];
    EXPECT_EQ(result0.home, "T1");
    EXPECT_EQ(result0.away, "T2");
    EXPECT_EQ(result0.home_goals, 0);
    EXPECT_EQ(result0.away_goals, 0);

    match result1 = s.summary()[1];
    EXPECT_EQ(result1.home, "T5");
    EXPECT_EQ(result1.away, "T6");
    EXPECT_EQ(result1.home_goals, 0);
    EXPECT_EQ(result1.away_goals, 0);
    
    match result2 = s.summary()[2];
    EXPECT_EQ(result2.home, "T7");
    EXPECT_EQ(result2.away, "T8");
    EXPECT_EQ(result2.home_goals, 0);
    EXPECT_EQ(result2.away_goals, 0);
}

TEST(SportRadar, FinishingAll)
{
    scoreboard s;
    s.start("T1", "T2");
    s.start("T3", "T4");
    s.start("T5", "T6");
    s.start("T7", "T8");
    s.finish("T1", "T2");
    s.finish("T3", "T4");
    s.finish("T5", "T6");
    s.finish("T7", "T8");
    
    EXPECT_EQ(s.summary(), std::vector<match>());
}

TEST(SportRadar, FinishingFirstOne)
{
    scoreboard s;
    s.start("T1", "T2");
    s.start("T3", "T4");
    s.start("T5", "T6");
    
    s.update({"T3", "T4", 1, 1});
    s.finish("T3", "T4");
    match result0 = s.summary()[0];
    EXPECT_EQ(result0.home, "T1");
    EXPECT_EQ(result0.away, "T2");
    EXPECT_EQ(result0.home_goals, 0);
    EXPECT_EQ(result0.away_goals, 0);

    match result1 = s.summary()[1];
    EXPECT_EQ(result1.home, "T5");
    EXPECT_EQ(result1.away, "T6");
    EXPECT_EQ(result1.home_goals, 0);
    EXPECT_EQ(result1.away_goals, 0);
}

TEST(SportRadar, FinishingLastOne)
{
    scoreboard s;
    s.start("T1", "T2");
    s.start("T3", "T4");
    s.start("T5", "T6");
    
    s.update({"T3", "T4", 1, 1});
    s.finish("T5", "T6");
    match result0 = s.summary()[0];
    EXPECT_EQ(result0.home, "T3");
    EXPECT_EQ(result0.away, "T4");
    EXPECT_EQ(result0.home_goals, 1);
    EXPECT_EQ(result0.away_goals, 1);

    match result1 = s.summary()[1];
    EXPECT_EQ(result1.home, "T1");
    EXPECT_EQ(result1.away, "T2");
    EXPECT_EQ(result1.home_goals, 0);
    EXPECT_EQ(result1.away_goals, 0);
}

TEST(SportRadar, Everything)
{
    scoreboard s;
    s.start("T1", "T2");
    s.start("T3", "T4");
    s.start("T5", "T6");
    s.start("T7", "T8");
    s.start("T9", "T10");
    s.start("T11", "T12");
    s.start("T13", "T14");
    s.update({ "T1", "T2", 1, 0 });
    s.update({ "T3", "T4", 1, 0 });
    s.update({ "T5", "T6", 2, 0 });
    s.update({ "T7", "T8", 3, 0 });

    {   // part1
        match result0 = s.summary()[0];
        EXPECT_EQ(result0.home, "T7");
        EXPECT_EQ(result0.away, "T8");
        EXPECT_EQ(result0.home_goals, 3);
        EXPECT_EQ(result0.away_goals, 0);
        
        match result1 = s.summary()[1];
        EXPECT_EQ(result1.home, "T5");
        EXPECT_EQ(result1.away, "T6");
        EXPECT_EQ(result1.home_goals, 2);
        EXPECT_EQ(result1.away_goals, 0);
        
        match result2 = s.summary()[2];
        EXPECT_EQ(result2.home, "T1");
        EXPECT_EQ(result2.away, "T2");
        EXPECT_EQ(result2.home_goals, 1);
        EXPECT_EQ(result2.away_goals, 0);
        
        match result3 = s.summary()[3];
        EXPECT_EQ(result3.home, "T3");
        EXPECT_EQ(result3.away, "T4");
        EXPECT_EQ(result3.home_goals, 1);
        EXPECT_EQ(result3.away_goals, 0);
        
        match result4 = s.summary()[4];
        EXPECT_EQ(result4.home, "T9");
        EXPECT_EQ(result4.away, "T10");
        EXPECT_EQ(result4.home_goals, 0);
        EXPECT_EQ(result4.away_goals, 0);
        
        match result5 = s.summary()[5];
        EXPECT_EQ(result5.home, "T11");
        EXPECT_EQ(result5.away, "T12");
        EXPECT_EQ(result5.home_goals, 0);
        EXPECT_EQ(result5.away_goals, 0);
        
        match result6 = s.summary()[6];
        EXPECT_EQ(result6.home, "T13");
        EXPECT_EQ(result6.away, "T14");
        EXPECT_EQ(result6.home_goals, 0);
        EXPECT_EQ(result6.away_goals, 0);
    }
    
    s.finish("T11", "T12");
    s.finish("T5", "T6");
    
    {   // part2
        match result0 = s.summary()[0];
        EXPECT_EQ(result0.home, "T7");
        EXPECT_EQ(result0.away, "T8");
        EXPECT_EQ(result0.home_goals, 3);
        EXPECT_EQ(result0.away_goals, 0);
        
        match result1 = s.summary()[1];
        EXPECT_EQ(result1.home, "T1");
        EXPECT_EQ(result1.away, "T2");
        EXPECT_EQ(result1.home_goals, 1);
        EXPECT_EQ(result1.away_goals, 0);
        
        match result2 = s.summary()[2];
        EXPECT_EQ(result2.home, "T3");
        EXPECT_EQ(result2.away, "T4");
        EXPECT_EQ(result2.home_goals, 1);
        EXPECT_EQ(result2.away_goals, 0);
        
        match result3 = s.summary()[3];
        EXPECT_EQ(result3.home, "T9");
        EXPECT_EQ(result3.away, "T10");
        EXPECT_EQ(result3.home_goals, 0);
        EXPECT_EQ(result3.away_goals, 0);
        
        match result4 = s.summary()[4];
        EXPECT_EQ(result4.home, "T13");
        EXPECT_EQ(result4.away, "T14");
        EXPECT_EQ(result4.home_goals, 0);
        EXPECT_EQ(result4.away_goals, 0);
    }
    
    s.update(match("T1", "T2", 1, 1));
    s.update(match("T9", "T10", 1, 0));
    s.update(match("T9", "T10", 2, 0));
    s.update(match("T9", "T10", 3, 0));
    s.update(match("T9", "T10", 4, 0));
    s.update(match("T9", "T10", 4, 1));
    s.update(match("T9", "T10", 5, 1));
    
    {   // part3
        match result0 = s.summary()[0];
        EXPECT_EQ(result0.home, "T9");
        EXPECT_EQ(result0.away, "T10");
        EXPECT_EQ(result0.home_goals, 5);
        EXPECT_EQ(result0.away_goals, 1);
        
        match result1 = s.summary()[1];
        EXPECT_EQ(result1.home, "T7");
        EXPECT_EQ(result1.away, "T8");
        EXPECT_EQ(result1.home_goals, 3);
        EXPECT_EQ(result1.away_goals, 0);
        
        match result2 = s.summary()[2];
        EXPECT_EQ(result2.home, "T1");
        EXPECT_EQ(result2.away, "T2");
        EXPECT_EQ(result2.home_goals, 1);
        EXPECT_EQ(result2.away_goals, 1);
        
        match result3 = s.summary()[3];
        EXPECT_EQ(result3.home, "T3");
        EXPECT_EQ(result3.away, "T4");
        EXPECT_EQ(result3.home_goals, 1);
        EXPECT_EQ(result3.away_goals, 0);
        
        match result4 = s.summary()[4];
        EXPECT_EQ(result4.home, "T13");
        EXPECT_EQ(result4.away, "T14");
        EXPECT_EQ(result4.home_goals, 0);
        EXPECT_EQ(result4.away_goals, 0);
    }
    
    s.finish("T7", "T8");
    s.start("T5", "T6");
    
    {   // part4
        match result0 = s.summary()[0];
        EXPECT_EQ(result0.home, "T9");
        EXPECT_EQ(result0.away, "T10");
        EXPECT_EQ(result0.home_goals, 5);
        EXPECT_EQ(result0.away_goals, 1);
        
        match result1 = s.summary()[1];
        EXPECT_EQ(result1.home, "T1");
        EXPECT_EQ(result1.away, "T2");
        EXPECT_EQ(result1.home_goals, 1);
        EXPECT_EQ(result1.away_goals, 1);
        
        match result2 = s.summary()[2];
        EXPECT_EQ(result2.home, "T3");
        EXPECT_EQ(result2.away, "T4");
        EXPECT_EQ(result2.home_goals, 1);
        EXPECT_EQ(result2.away_goals, 0);
        
        match result3 = s.summary()[3];
        EXPECT_EQ(result3.home, "T13");
        EXPECT_EQ(result3.away, "T14");
        EXPECT_EQ(result3.home_goals, 0);
        EXPECT_EQ(result3.away_goals, 0);
        
        match result4 = s.summary()[4];
        EXPECT_EQ(result4.home, "T5");
        EXPECT_EQ(result4.away, "T6");
        EXPECT_EQ(result4.home_goals, 0);
        EXPECT_EQ(result4.away_goals, 0);
    }
}
