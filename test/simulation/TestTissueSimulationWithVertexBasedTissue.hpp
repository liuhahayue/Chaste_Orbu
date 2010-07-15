/*

Copyright (C) University of Oxford, 2005-2010

University of Oxford means the Chancellor, Masters and Scholars of the
University of Oxford, having an administrative office at Wellington
Square, Oxford OX1 2JD, UK.

This file is part of Chaste.

Chaste is free software: you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published
by the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Chaste is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details. The offer of Chaste under the terms of the
License is subject to the License being interpreted in accordance with
English Law and subject to any action against the University of Oxford
being under the jurisdiction of the English Courts.

You should have received a copy of the GNU Lesser General Public License
along with Chaste. If not, see <http://www.gnu.org/licenses/>.

*/
#ifndef TESTTISSUESIMULATIONWITHVERTEXBASEDTISSUE_HPP_
#define TESTTISSUESIMULATIONWITHVERTEXBASEDTISSUE_HPP_

#include <cxxtest/TestSuite.h>

// Must be included before other cell_based headers
#include "TissueSimulationArchiver.hpp"

#include "TissueSimulation.hpp"
#include "FixedDurationGenerationBasedCellCycleModel.hpp"
#include "StochasticDurationGenerationBasedCellCycleModel.hpp"
#include "VertexBasedTissue.hpp"
#include "NagaiHondaForce.hpp"
#include "WelikyOsterForce.hpp"
#include "AbstractCellKiller.hpp"
#include "AbstractCellBasedTestSuite.hpp"
#include "HoneycombMutableVertexMeshGenerator.hpp"
#include "VertexMeshWriter.hpp"
#include "WildTypeCellMutationState.hpp"
//#include "LabelledCellMutationState.hpp"

#include "Warnings.hpp"

/**
 * Simple cell killer which at the first timestep kills any cell
 * whose corresponding location index is a given number.
 *
 * For testing purposes.
 */
class TargetedCellKiller : public AbstractCellKiller<2>
{
private :

    unsigned mTargetIndex;
    bool mBloodLust;

public :
    TargetedCellKiller(AbstractTissue<2>* pTissue, unsigned targetIndex)
        : AbstractCellKiller<2>(pTissue),
          mTargetIndex(targetIndex),
          mBloodLust(true)
    {
    }

    virtual void TestAndLabelCellsForApoptosisOrDeath()
    {
        if ( !mBloodLust || mpTissue->GetNumRealCells()==0 || mpTissue->GetNumRealCells()<mTargetIndex+1)
        {
            return;
        }
        mpTissue->GetCellUsingLocationIndex(mTargetIndex)->Kill();
        mBloodLust = false;
    }
};


class TestTissueSimulationWithVertexBasedTissue : public AbstractCellBasedTestSuite
{
private:

    double mLastStartTime;
    void setUp()
    {
        mLastStartTime = std::clock();
        AbstractCellBasedTestSuite::setUp();
    }
    void tearDown()
    {
        double time = std::clock();
        double elapsed_time = (time - mLastStartTime)/(CLOCKS_PER_SEC);
        std::cout << "Elapsed time: " << elapsed_time << std::endl;
        AbstractCellBasedTestSuite::tearDown();
    }

public:

    void TestSingleCellRelaxation() throw (Exception)
    {
        // Construct a 2D vertex mesh consisting of a single element
        std::vector<Node<2>*> nodes;
        unsigned num_nodes = 20;
        for (unsigned i=0; i<num_nodes; i++)
        {
            double theta = M_PI+2.0*M_PI*(double)(i)/(double)(num_nodes);
            nodes.push_back(new Node<2>(i, false, cos(theta), sin(theta)));
        }

        std::vector<VertexElement<2,2>*> elements;
        elements.push_back(new VertexElement<2,2>(0, nodes));

        double cell_swap_threshold = 0.01;
        double edge_division_threshold = 2.0;
        MutableVertexMesh<2,2> mesh(nodes, elements, cell_swap_threshold, edge_division_threshold);

        // Set up cells, one for each VertexElement. Give each cell
        // a birth time of 0
        std::vector<TissueCellPtr> cells;
        boost::shared_ptr<AbstractCellMutationState> p_state(new WildTypeCellMutationState);
        for (unsigned elem_index=0; elem_index<mesh.GetNumElements(); elem_index++)
        {
            FixedDurationGenerationBasedCellCycleModel* p_model = new FixedDurationGenerationBasedCellCycleModel();
            p_model->SetCellProliferativeType(DIFFERENTIATED);

            TissueCellPtr p_cell(new TissueCell(p_state, p_model));
            double birth_time = -1.0;
            p_cell->SetBirthTime(birth_time);
            cells.push_back(p_cell);
        }

        // Create tissue
        VertexBasedTissue<2> tissue(mesh, cells);

        // Create a force system
        NagaiHondaForce<2> force;
        std::vector<AbstractForce<2>* > force_collection;
        force_collection.push_back(&force);

        // Set up tissue simulation
        TissueSimulation<2> simulator(tissue, force_collection);
        simulator.SetOutputDirectory("TestSingleCellRelaxation");
        simulator.SetEndTime(1.0);

        // Run simulation
        simulator.Solve();

        // Test relaxes to circle (can be more stringent with more nodes and more time)
        TS_ASSERT_DELTA(tissue.rGetMesh().GetVolumeOfElement(0), 1.0, 0.05);
        TS_ASSERT_DELTA(tissue.rGetMesh().GetSurfaceAreaOfElement(0), 3.5449077, 0.1);
    }

    void TestSingleCellRelaxationWelikyOster() throw (Exception)
    {
        // Construct a 2D vertex mesh consisting of a single element
        std::vector<Node<2>*> nodes;
        unsigned num_nodes = 20;
        for (unsigned i=0; i<num_nodes; i++)
        {
            double theta = M_PI+2.0*M_PI*(double)(i)/(double)(num_nodes);
            nodes.push_back(new Node<2>(i, false, cos(theta), sin(theta)));
        }

        std::vector<VertexElement<2,2>*> elements;
        elements.push_back(new VertexElement<2,2>(0, nodes));

        double cell_swap_threshold = 0.01;
        double edge_division_threshold = 2.0;
        MutableVertexMesh<2,2> mesh(nodes, elements, cell_swap_threshold, edge_division_threshold);

        // Set up cells, one for each VertexElement. Give each cell
        // a birth time of 0
        std::vector<TissueCellPtr> cells;
        boost::shared_ptr<AbstractCellMutationState> p_state(new WildTypeCellMutationState);
        for (unsigned elem_index=0; elem_index<mesh.GetNumElements(); elem_index++)
        {
            FixedDurationGenerationBasedCellCycleModel* p_model = new FixedDurationGenerationBasedCellCycleModel();
            p_model->SetCellProliferativeType(DIFFERENTIATED);

            TissueCellPtr p_cell(new TissueCell(p_state, p_model));
            p_cell->SetBirthTime(-1.0);
            cells.push_back(p_cell);
        }

        // Create tissue
        VertexBasedTissue<2> tissue(mesh, cells);

        // Create a force system
        WelikyOsterForce<2> force;
        std::vector<AbstractForce<2>* > force_collection;
        force_collection.push_back(&force);

        // Set up tissue simulation
        TissueSimulation<2> simulator(tissue, force_collection);
        simulator.SetOutputDirectory("TestSingleCellRelaxationWelikyOster");
        simulator.SetEndTime(1.0);

        // Run simulation
        simulator.Solve();

        // Test relaxes to circle (can be more stringent with more nodes and more time)
        TS_ASSERT_DELTA(tissue.rGetMesh().GetVolumeOfElement(0), 1.0, 0.1);
        TS_ASSERT_DELTA(tissue.rGetMesh().GetSurfaceAreaOfElement(0), 3.5449077, 0.1);
    }

    // In this test edges can divide if they become too long.
    void TestSimpleVertexMonolayerWithCellBirth() throw (Exception)
    {
        // Create a simple 2D MutableVertexMesh with only one cell
        HoneycombMutableVertexMeshGenerator generator(1, 1);
        MutableVertexMesh<2,2>* p_mesh = generator.GetMutableMesh();
        p_mesh->SetEdgeDivisionThreshold(0.5); // See ticket # 1477

        // Set up cell.
        std::vector<TissueCellPtr> cells;
        boost::shared_ptr<AbstractCellMutationState> p_state(new WildTypeCellMutationState);

        FixedDurationGenerationBasedCellCycleModel* p_model = new FixedDurationGenerationBasedCellCycleModel();
        p_model->SetCellProliferativeType(TRANSIT);

        TissueCellPtr p_cell(new TissueCell(p_state, p_model));
        double birth_time = -20.0; // divides straight away
        p_cell->SetBirthTime(birth_time);
        cells.push_back(p_cell);

        // Create tissue
        VertexBasedTissue<2> tissue(*p_mesh, cells);

        unsigned old_num_nodes = tissue.GetNumNodes();
        unsigned old_num_elements = tissue.GetNumElements();
        unsigned old_num_cells = tissue.GetNumRealCells();

        // Create a force system
        NagaiHondaForce<2> force;
        std::vector<AbstractForce<2>* > force_collection;
        force_collection.push_back(&force);

        // Set up tissue simulation
        TissueSimulation<2> simulator(tissue, force_collection);
        simulator.SetOutputDirectory("TestSimpleVertexMonolayerWithCellBirth");
        simulator.SetEndTime(1.0);

        // Run simulation
        simulator.Solve();

        // Check that cell divided successfully
        unsigned new_num_nodes = simulator.rGetTissue().GetNumNodes();
        unsigned new_num_elements = (static_cast<VertexBasedTissue<2>*>(&(simulator.rGetTissue())))->GetNumElements();
        unsigned new_num_cells = simulator.rGetTissue().GetNumRealCells();

        TS_ASSERT_EQUALS(new_num_nodes, old_num_nodes+9); // as division of element is longer than threshold so is divided
        TS_ASSERT_EQUALS(new_num_elements, old_num_elements+1);
        TS_ASSERT_EQUALS(new_num_cells, old_num_cells+1);
    }

    // This test visualizing cells of 2 mutation types, wildtype and labelled type.
    // It asserts that neighboring cells have the correct adhesion parameter for difference
    // pairs of nodes.
	void TestSimpleVertexMonolayerWithTwoMutationTypes() throw (Exception)
	{
		TissueConfig::Instance()->SetOutputCellMutationStates(true);

		// Create a simple 2D MutableVertexMesh with only four cells
		HoneycombMutableVertexMeshGenerator generator(2, 2);
		MutableVertexMesh<2,2>* p_mesh = generator.GetMutableMesh();

		// Set up cell.
		std::vector<TissueCellPtr> cells;
		boost::shared_ptr<AbstractCellMutationState> p_state1(new WildTypeCellMutationState);
		boost::shared_ptr<AbstractCellMutationState> p_state2(new LabelledCellMutationState);

		for (unsigned elem_index=0; elem_index<p_mesh->GetNumElements(); elem_index++)
		{
			FixedDurationGenerationBasedCellCycleModel* p_model = new FixedDurationGenerationBasedCellCycleModel();
			p_model->SetCellProliferativeType(TRANSIT);

			if (elem_index == 0)
			{
				TissueCellPtr p_cell1(new TissueCell(p_state2, p_model));
				double birth_time = -2.0;
				p_cell1->SetBirthTime(birth_time);
				TS_ASSERT_EQUALS(p_cell1->GetMutationState()->IsType<LabelledCellMutationState>(), true);
				cells.push_back(p_cell1);
			}
			else
			{
				TissueCellPtr p_cell(new TissueCell(p_state1, p_model));
				double birth_time = -2.0;
				p_cell->SetBirthTime(birth_time);
				cells.push_back(p_cell);
			}
		}
		// Create tissue
		VertexBasedTissue<2> tissue(*p_mesh, cells);

		// Create a force system
		NagaiHondaForce<2> force;
		std::vector<AbstractForce<2>* > force_collection;
		force_collection.push_back(&force);

		// Set up tissue simulation
		TissueSimulation<2> simulator(tissue, force_collection);
		simulator.SetOutputDirectory("TestSimpleVertexMonolayerWithTwoMutationStates");
		simulator.SetEndTime(1.0);

		// Run simulation
		simulator.Solve();
		TS_ASSERT_DELTA(force.GetAdhesionParameter(p_mesh->GetNode(3), p_mesh->GetNode(6), 2), 1.5, 1e-4);
		TS_ASSERT_DELTA(force.GetAdhesionParameter(p_mesh->GetNode(6), p_mesh->GetNode(3), 2), 1.5, 1e-4);
		TS_ASSERT_DELTA(force.GetAdhesionParameter(p_mesh->GetNode(9), p_mesh->GetNode(12), 0), 1.0, 1e-4);

		TS_ASSERT_EQUALS(force.GetCombinationCellTypes(p_mesh->GetNode(3), p_mesh->GetNode(6), tissue), 2u);
		TS_ASSERT_EQUALS(force.GetCombinationCellTypes(p_mesh->GetNode(6), p_mesh->GetNode(3), tissue), 2u);
		TS_ASSERT_EQUALS(force.GetCombinationCellTypes(p_mesh->GetNode(9), p_mesh->GetNode(12), tissue), 0u);

		TS_ASSERT_EQUALS(p_mesh->GetNode(13)->IsBoundaryNode(), true);
		TS_ASSERT_EQUALS(p_mesh->GetNumElements(),4u);
		TS_ASSERT_EQUALS(p_state2->GetColour(), 5u);
		TS_ASSERT_EQUALS(p_state1->GetColour(), 0u);
		TS_ASSERT_EQUALS(cells[0]->GetMutationState()->IsType<LabelledCellMutationState>(), true);
	}

    void TestVertexMonolayerWithCellBirth() throw (Exception)
    {
        // Create a simple 2D MutableVertexMesh
        HoneycombMutableVertexMeshGenerator generator(5, 5);
        MutableVertexMesh<2,2>* p_mesh = generator.GetMutableMesh();

        // Set up cells, one for each VertexElement. Give each cell
        // a birth time of -elem_index, so its age is elem_index
        std::vector<TissueCellPtr> cells;
        boost::shared_ptr<AbstractCellMutationState> p_state(new WildTypeCellMutationState);
        for (unsigned elem_index=0; elem_index<p_mesh->GetNumElements(); elem_index++)
        {
            CellProliferativeType cell_type = DIFFERENTIATED;
            double birth_time = 0.0 - elem_index;

            // Cell should divide at time t=0.05
            if (elem_index==12) //(elem_index==6 ||elem_index==7 || elem_index==8 || elem_index==11 ||elem_index==12 || elem_index==13 ||elem_index==16 ||elem_index==17 || elem_index==18)
            {
                cell_type = STEM;
                birth_time = -23.95;
            }

            FixedDurationGenerationBasedCellCycleModel* p_model = new FixedDurationGenerationBasedCellCycleModel();
            p_model->SetCellProliferativeType(cell_type);

            TissueCellPtr p_cell(new TissueCell(p_state, p_model));
            p_cell->SetBirthTime(birth_time);
            cells.push_back(p_cell);
        }

        // Create tissue
        VertexBasedTissue<2> tissue(*p_mesh, cells);

        unsigned old_num_nodes = tissue.GetNumNodes();
        unsigned old_num_elements = tissue.GetNumElements();
        unsigned old_num_cells = tissue.GetNumRealCells();

        // Create a force system
        NagaiHondaForce<2> force;
        std::vector<AbstractForce<2>* > force_collection;
        force_collection.push_back(&force);

        // Set up tissue simulation
        TissueSimulation<2> simulator(tissue, force_collection);
        simulator.SetOutputDirectory("TestVertexMonolayerWithCellBirth");
        simulator.SetSamplingTimestepMultiple(50);
        simulator.SetEndTime(0.1);

        // Run simulation
        simulator.Solve();

        // Check that cell 12 divided successfully
        unsigned new_num_nodes = simulator.rGetTissue().GetNumNodes();
        unsigned new_num_elements = (static_cast<VertexBasedTissue<2>*>(&(simulator.rGetTissue())))->GetNumElements();
        unsigned new_num_cells = simulator.rGetTissue().GetNumRealCells();

        TS_ASSERT_EQUALS(new_num_nodes, old_num_nodes+2); // as division of element is longer than threshold so is divided
        TS_ASSERT_EQUALS(new_num_elements, old_num_elements+1);
        TS_ASSERT_EQUALS(new_num_cells, old_num_cells+1);
    }


    /*
     * This test will fail with the larger timestep unless the movement is restricted to less than mCellRearangementThreshold.
     */
    void noTestVertexMonolayerWithVoid() throw (Exception)
    {
        // Create a simple 2D MutableVertexMesh
        HoneycombMutableVertexMeshGenerator generator(3, 3);
        MutableVertexMesh<2,2>* p_mesh = generator.GetMutableMesh();

        // create a hole in the mesh
        p_mesh->DeleteElementPriorToReMesh(0);
        p_mesh->DeleteElementPriorToReMesh(4);
        p_mesh->DeleteElementPriorToReMesh(6);

        p_mesh->ReMesh();

        // Set up cells, one for each VertexElement. Give each cell
        // a birth time of -elem_index-19, so its age is elem_index+19, so the first cell divides straight away.
        std::vector<TissueCellPtr> cells;
        boost::shared_ptr<AbstractCellMutationState> p_state(new WildTypeCellMutationState);
        for (unsigned elem_index=0; elem_index<p_mesh->GetNumElements(); elem_index++)
        {
            FixedDurationGenerationBasedCellCycleModel* p_model = new FixedDurationGenerationBasedCellCycleModel();
            p_model->SetCellProliferativeType(STEM);

            TissueCellPtr p_cell(new TissueCell(p_state, p_model));
            double birth_time;
            birth_time = -(double)elem_index -19.0;
            p_cell->SetBirthTime(birth_time);
            cells.push_back(p_cell);
        }

        // Create tissue
        VertexBasedTissue<2> tissue(*p_mesh, cells);

        // Create a force system
        NagaiHondaForce<2> force;
        std::vector<AbstractForce<2>* > force_collection;
        force_collection.push_back(&force);

        // Set up tissue simulation
        TissueSimulation<2> simulator(tissue, force_collection);
        simulator.SetOutputDirectory("TestVertexMonolayerWithVoid");
        simulator.SetEndTime(30.0);

        ////////////////////////////////////////////
        /// Strange setup to speed up simulation ///
        ////////////////////////////////////////////
//        p_mesh->SetCellRearrangementThreshold(0.05);
//        simulator.SetDt(0.1);
        ////////////////////////////////////////////

        // Run simulation
        simulator.Solve();

        // Check that void has been removed and vertices are in the correct position
        TS_ASSERT_EQUALS(simulator.rGetTissue().GetNumNodes(),82u);
        TS_ASSERT_EQUALS((static_cast<VertexBasedTissue<2>*>(&(simulator.rGetTissue())))->GetNumElements(),36u);
        TS_ASSERT_EQUALS(simulator.rGetTissue().GetNumRealCells(),36u);

    }

    void TestVertexMonolayerWithCellDeath() throw (Exception)
    {
        /*
         * We don't want apoptosing cells to be labelled as dead after a certain time in
         * vertex simulations, so set the apoptosis time to something large.
         */

        // Create a simple 2D MutableVertexMesh
        HoneycombMutableVertexMeshGenerator generator(4,4);
        MutableVertexMesh<2,2>* p_mesh = generator.GetMutableMesh();
        p_mesh->SetCellRearrangementThreshold(0.1);
        p_mesh->SetT2Threshold(1.0); // so T2Swaps once it becomes a triangle

        // Set up cells, one for each VertexElement. Give each cell
        // a birth time of -elem_index, so its age is elem_index
        std::vector<TissueCellPtr> cells;
        boost::shared_ptr<AbstractCellMutationState> p_state(new WildTypeCellMutationState);
        for (unsigned elem_index=0; elem_index<p_mesh->GetNumElements(); elem_index++)
        {
            FixedDurationGenerationBasedCellCycleModel* p_model = new FixedDurationGenerationBasedCellCycleModel();
            p_model->SetCellProliferativeType(DIFFERENTIATED);

            TissueCellPtr p_cell(new TissueCell(p_state, p_model));
            double birth_time = 0.0 - elem_index;
            p_cell->SetBirthTime(birth_time);

            if (elem_index==6)
            {
                p_cell->StartApoptosis(false);
            }

            if (elem_index==14)
			{
				p_cell->StartApoptosis(true);
			}

            cells.push_back(p_cell);
        }

        // Create tissue
        VertexBasedTissue<2> tissue(*p_mesh, cells);

        unsigned old_num_nodes = tissue.GetNumNodes();
        unsigned old_num_elements = tissue.GetNumElements();
        unsigned old_num_cells = tissue.GetNumRealCells();

        // Create a force system
        NagaiHondaForce<2> force;
        std::vector<AbstractForce<2>* > force_collection;
        force_collection.push_back(&force);

        // Set up tissue simulation
        TissueSimulation<2> simulator(tissue, force_collection);
        simulator.SetOutputDirectory("TestVertexMonolayerWithCellDeath");
        simulator.SetEndTime(0.5);
        
        // Create a cell killer and pass in to simulation (note we must account for element index changes following each kill)
        TargetedCellKiller cell0_killer(&tissue, 0);    // element on the SW corner
        TargetedCellKiller cell2_killer(&tissue, 2);    // element on the S boundary
        TargetedCellKiller cell9_killer(&tissue, 9);  // element on the interior

        simulator.AddCellKiller(&cell0_killer);
        simulator.AddCellKiller(&cell2_killer);
        simulator.AddCellKiller(&cell9_killer);

        // Run simulation
        simulator.Solve();
        TS_ASSERT_EQUALS(Warnings::Instance()->GetNextWarningMessage(),"Cell removed due to T2Swap this is not counted in the dead cells counter");

        // Check that cells 6 and 14 have now been removed.
        unsigned new_num_nodes = simulator.rGetTissue().GetNumNodes();
        unsigned new_num_elements = (static_cast<VertexBasedTissue<2>*>(&(simulator.rGetTissue())))->GetNumElements();
        unsigned new_num_cells = simulator.rGetTissue().GetNumRealCells();

        TS_ASSERT_EQUALS(new_num_nodes, old_num_nodes-7);	// Due to the cells on the boundary that get killed and the apoptotic cell that does a T2 swap
        TS_ASSERT_EQUALS(new_num_elements, old_num_elements-5);
        TS_ASSERT_EQUALS(new_num_cells, old_num_cells-5);
        TS_ASSERT_EQUALS(new_num_cells, new_num_elements);

        Warnings::QuietDestroy();
    }

    void TestSingleCellRelaxationAndApoptosis() throw (Exception)
    {
        // Construct a 2D vertex mesh consisting of a single element
        std::vector<Node<2>*> nodes;
        unsigned num_nodes = 20;
        for (unsigned i=0; i<num_nodes; i++)
        {
            double theta = M_PI+2.0*M_PI*(double)(i)/(double)(num_nodes);
            nodes.push_back(new Node<2>(i, false, cos(theta), sin(theta)));
        }

        std::vector<VertexElement<2,2>*> elements;
        elements.push_back(new VertexElement<2,2>(0, nodes));

        MutableVertexMesh<2,2> mesh(nodes, elements);
        mesh.SetCellRearrangementThreshold(0.1);

        // Set up cells, one for each VertexElement
        std::vector<TissueCellPtr> cells;
        boost::shared_ptr<AbstractCellMutationState> p_state(new WildTypeCellMutationState);
        for (unsigned elem_index=0; elem_index<mesh.GetNumElements(); elem_index++)
        {
            FixedDurationGenerationBasedCellCycleModel* p_model = new FixedDurationGenerationBasedCellCycleModel();
            p_model->SetCellProliferativeType(DIFFERENTIATED);

            TissueCellPtr p_cell(new TissueCell(p_state, p_model));
            double birth_time = -1.0;
            p_cell->SetBirthTime(birth_time);
            cells.push_back(p_cell);
        }

        // Create tissue
        VertexBasedTissue<2> tissue(mesh, cells);

        // Create a force system
        NagaiHondaForce<2> force;
        std::vector<AbstractForce<2>* > force_collection;
        force_collection.push_back(&force);

        // Set up tissue simulation
        TissueSimulation<2> simulator(tissue, force_collection);
        simulator.SetOutputDirectory("TestVertexSingleCellApoptosis");
        simulator.SetEndTime(2.0);

        // Run simulation
        simulator.Solve();

        // Test relaxes to circle (can be more stringent with more nodes and more time)
        TS_ASSERT_DELTA(tissue.rGetMesh().GetVolumeOfElement(0), 1.0, 1e-1);
        TS_ASSERT_DELTA(tissue.rGetMesh().GetSurfaceAreaOfElement(0), 3.5449077, 1e-1);

        TissueCellPtr p_cell = simulator.rGetTissue().GetCellUsingLocationIndex(0);
        p_cell->StartApoptosis(false);

        simulator.SetEndTime(2.25); // any longer and cell target area is zero but element can't be removed as its the only one.

        // Run simulation
        simulator.Solve();

        TS_ASSERT_DELTA(tissue.rGetMesh().GetVolumeOfElement(0), 0.5098, 1e-4);
        TS_ASSERT_DELTA(tissue.rGetMesh().GetSurfaceAreaOfElement(0), 2.5417, 1e-3);
    }


    /*
     * This test is to stress test the vertex simulations by creating a massive monolayer,
     * it would also be useful for benchmarking.
     *
     * \todo In order to work the mesh archiving must support boundary nodes, see #1076
     */
    void noTestVertexStressTest() throw (Exception)
    {
        double start_time=0.0;
        double end_time=100.0;
        std::string output_directory = "StressTestVertex";

        // Create a simple 2D MutableVertexMesh
        HoneycombMutableVertexMeshGenerator generator(3, 3);
        MutableVertexMesh<2,2>* p_mesh = generator.GetMutableMesh();

        // Set up cells, one for each VertexElement. Give each cell
        // a birth time of -elem_index, so its age is elem_index
        std::vector<TissueCellPtr> cells;
        boost::shared_ptr<AbstractCellMutationState> p_state(new WildTypeCellMutationState);
        for (unsigned elem_index=0; elem_index<p_mesh->GetNumElements(); elem_index++)
        {
            StochasticDurationGenerationBasedCellCycleModel* p_model = new StochasticDurationGenerationBasedCellCycleModel();
            p_model->SetCellProliferativeType(STEM);

            TissueCellPtr p_cell(new TissueCell(p_state, p_model));
            double birth_time = -(double)elem_index;
            p_cell->SetBirthTime(birth_time);
            cells.push_back(p_cell);
        }

        // Create tissue
        VertexBasedTissue<2> tissue(*p_mesh, cells);

        // Create a force system
        NagaiHondaForce<2> force;
        std::vector<AbstractForce<2>* > force_collection;
        force_collection.push_back(&force);

        // Set up tissue simulation
        TissueSimulation<2> simulator(tissue, force_collection);

        simulator.SetOutputDirectory(output_directory);
        simulator.SetSamplingTimestepMultiple(50);

        simulator.SetEndTime(end_time);

        // Run simulation
        simulator.Solve();

        //archive now and then reload

        // Save simulation in steady state
        TissueSimulationArchiver<2, TissueSimulation<2> >::Save(&simulator);

        //Now save and reload to find where it breaks!
        for (int i=0; i<40; i++)
        {
            start_time = end_time;
            end_time = end_time + 10.0;

            TissueSimulation<2>* p_simulator = TissueSimulationArchiver<2, TissueSimulation<2> >::Load(output_directory,start_time);
            p_simulator->SetDt(0.002);
            p_simulator->SetSamplingTimestepMultiple(50);
            p_simulator->SetEndTime(end_time);
            p_simulator->Solve();

            TissueSimulationArchiver<2, TissueSimulation<2> >::Save(p_simulator);
            delete p_simulator;
        }
    }


    /**
     * Test archiving of a TissueSimulation that uses a VertexBasedTissue.
     */
    void TestArchiving() throw (Exception)
    {
        // Set end time
        double end_time = 0.1;

        // Create a simple 2D MutableVertexMesh
        HoneycombMutableVertexMeshGenerator generator(6, 6);
        MutableVertexMesh<2,2>* p_mesh = generator.GetMutableMesh();

        // Set up cells, one for each VertexElement. Give each cell
        // a birth time of -elem_index, so its age is elem_index
        std::vector<TissueCellPtr> cells;
        boost::shared_ptr<AbstractCellMutationState> p_state(new WildTypeCellMutationState);
        for (unsigned elem_index=0; elem_index<p_mesh->GetNumElements(); elem_index++)
        {
            FixedDurationGenerationBasedCellCycleModel* p_model = new FixedDurationGenerationBasedCellCycleModel();
            p_model->SetCellProliferativeType(DIFFERENTIATED);

            TissueCellPtr p_cell(new TissueCell(p_state, p_model));
            double birth_time = 0.0 - elem_index;
            p_cell->SetBirthTime(birth_time);
            cells.push_back(p_cell);
        }

        // Create tissue
        VertexBasedTissue<2> tissue(*p_mesh, cells);

        // Create a force system
        NagaiHondaForce<2> force;
        std::vector<AbstractForce<2>* > force_collection;
        force_collection.push_back(&force);

        // Set up tissue simulation
        TissueSimulation<2> simulator(tissue, force_collection);
        simulator.SetOutputDirectory("TestTissueSimulationWithVertexBasedTissueSaveAndLoad");
        simulator.SetEndTime(end_time);

        TS_ASSERT_DELTA(simulator.GetDt(), 0.002, 1e-12);

        // Run and save simulation
        TS_ASSERT_THROWS_NOTHING(simulator.Solve());

        TissueSimulationArchiver<2, TissueSimulation<2> >::Save(&simulator);

        TS_ASSERT_EQUALS(simulator.rGetTissue().GetNumRealCells(), 36u);
        TS_ASSERT_EQUALS(simulator.rGetTissue().GetNumNodes(), 96u);
        TS_ASSERT_EQUALS((static_cast<VertexBasedTissue<2>*>(&(simulator.rGetTissue())))->GetNumElements(), 36u);

        TS_ASSERT_DELTA(SimulationTime::Instance()->GetTime(), 0.1, 1e-9);
        TissueCellPtr p_cell = simulator.rGetTissue().GetCellUsingLocationIndex(23);
        TS_ASSERT_DELTA(p_cell->GetAge(), 23.1, 1e-4);

        SimulationTime::Destroy();
        SimulationTime::Instance()->SetStartTime(0.0);

        // Load simulation
        TissueSimulation<2>* p_simulator
            = TissueSimulationArchiver<2, TissueSimulation<2> >::Load("TestTissueSimulationWithVertexBasedTissueSaveAndLoad", end_time);

        p_simulator->SetEndTime(0.2);

        TS_ASSERT_EQUALS(p_simulator->rGetTissue().GetNumRealCells(), 36u);
        TS_ASSERT_EQUALS(p_simulator->rGetTissue().GetNumNodes(), 96u);
        TS_ASSERT_EQUALS((static_cast<VertexBasedTissue<2>*>(&(p_simulator->rGetTissue())))->GetNumElements(), 36u);

        TS_ASSERT_DELTA(SimulationTime::Instance()->GetTime(), 0.1, 1e-9);
        TissueCellPtr p_cell2 = p_simulator->rGetTissue().GetCellUsingLocationIndex(23);
        TS_ASSERT_DELTA(p_cell2->GetAge(), 23.1, 1e-4);

        // Run simulation
        TS_ASSERT_THROWS_NOTHING(p_simulator->Solve());

        // Tidy up
        delete p_simulator;
    }
};

#endif /*TESTTISSUESIMULATIONWITHVERTEXBASEDTISSUE_HPP_*/
