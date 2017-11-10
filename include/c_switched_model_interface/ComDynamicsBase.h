/*
 * ComDynamicsBase.h
 *
 *  Created on: Nov 7, 2017
 *      Author: farbod
 */

#ifndef COMDYNAMICSBASE_H_
#define COMDYNAMICSBASE_H_

#include <array>
#include <memory>
#include <iostream>
#include <Eigen/Dense>

#include <dynamics/ControlledSystemBase.h>

#include "SwitchedModel.h"
#include "KinematicsModelBase.h"
#include "ComModelBase.h"

template <size_t JOINT_COORD_SIZE>
class ComDynamicsBase : public ocs2::ControlledSystemBase<12, 12>
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	typedef ocs2::ControlledSystemBase<12,12> Base;
	typedef ComModelBase<JOINT_COORD_SIZE>    		com_model_t;
	typedef KinematicsModelBase<JOINT_COORD_SIZE>   kinematic_model_t;

	typedef typename SwitchedModel<JOINT_COORD_SIZE>::base_coordinate_t  base_coordinate_t;
	typedef typename SwitchedModel<JOINT_COORD_SIZE>::joint_coordinate_t joint_coordinate_t;


	ComDynamicsBase(const kinematic_model_t& kinematicModel, const com_model_t& comModel,
			const double& gravitationalAcceleration=9.81, const bool& constrainedIntegration=true)

	: comModel_(comModel),
	  o_gravityVector_(0.0, 0.0, -gravitationalAcceleration),
	  constrainedIntegration_(constrainedIntegration)
	{
		if (gravitationalAcceleration<0)  throw std::runtime_error("Gravitational acceleration should be a positive value.");
	}

	virtual ~ComDynamicsBase() {}

	/**
	 * clone this class.
	 */
	virtual std::shared_ptr<Base> clone() const  override;

	/**
	 * Initializes the model: This method should always be called at the very first call of the model.
	 */
	virtual void initializeModel(const std::vector<size_t>& systemStockIndexes, const std::vector<scalar_t>& switchingTimes,
			const state_vector_t& initState, const size_t& activeSubsystemIndex=0, const char* algorithmName=NULL) override;

	/**
	 * Calculates the CoM state time evolution based on the current CoM state (x), contact forces input
	 * at all feet (u), the stance leg configuration (stanceLegs_), the current joints' angel (qJoints_),
	 * and the current joints' velocities (dqJoints_).
	 *
	 * The CoM state (x) consists of:
	 * 		+ Base orientation w.r.t Origin frame (3-states)
	 * 		+ CoM position w.r.t Origin frame (3-states)
	 * 		+ CoM local angular and linear velocities in CoM frame (inertia frame coincide at CoM and is
	 * 		  parallel to Base frame) (6-states)
	 *
	 * The control input (u) consists of:
	 * 		+ feet's contact forces in the CoM frame with spherical coordinate (12-inputs = 3-XYZ-forces * 4-feet)
	 *
	 * The CoM state time derivatives (dxdt) consists of:
	 * 		+ Base angular velocity w.r.t Origin frame (3-states)
	 * 		+ CoM linear velocity w.r.t Origin frame (3-states)
	 * 		+ CoM angular and linear accelerations w.r.t CoM frame (6-states)
	 *
	 */
	virtual void computeDerivative(const scalar_t& t,
			const state_vector_t& x,
			const control_vector_t& u,
			state_vector_t& dxdt) override;

	/**
	 * set joints' angel and angular velocity, and stance leg configuration. This data is
	 * required in computeDerivative() method.
	 */
	void setData(const std::array<bool,4>& stanceLegs,
			const joint_coordinate_t& qJoints,
			const joint_coordinate_t& dqJoints);

	/**
	 * Calculates the Base orientation and position based on the current joints' angel (qJoints)
	 * and the current CoM orientation and position.
	 *
	 * The Base pose (basePose) consists of:
	 * 		+ Base orientation w.r.t origin frame (3-states)
	 * 		+ Base position w.r.t origin frame (3-states)
	 */
	static void CalculateBasePose(const joint_coordinate_t& qJoints,
			const base_coordinate_t& comPose,
			base_coordinate_t& basePose);

	/**
	 * Calculates the Base local velocities based on the current joints' angel (qJoints),
	 * the current joints' velocities (dqJoints), CoM local velocities (comLocalVelocities).
	 *
	 * The Base local velocities (baseLocalVelocities) consists of:
	 * 		+ Base local angular and linear velocities in World frame (inertia frame coincide at Base frame) (6-states)
	 */
	static void CalculateBaseLocalVelocities(const joint_coordinate_t& qJoints,
			const joint_coordinate_t& dqJoints,
			const base_coordinate_t& comLocalVelocities,
			base_coordinate_t& baseLocalVelocities);

	/**
	 * @brief Computes the matrix which transforms derivatives of angular velocities in the body frame to euler angles derivatives
	 * WARNING: matrix is singular when rotation around y axis is +/- 90 degrees
	 * @param[in] eulerAngles: euler angles in xyz convention
	 * @return M: matrix that does the transformation
	 */
	static Eigen::Matrix3d AngularVelocitiesToEulerAngleDerivativesMatrix (Eigen::Vector3d eulerAngles);

	/**
	 * user interface for retrieving stance legs. Note this value is updated after each call of computeDerivative() method.
	 */
	void getStanceLegs(std::array<bool,4>& stanceLegs) const;

	/**
	 * user interface for retrieving feet's positions. Note this value is updated after each call of computeDerivative() method.
	 */
	void getFeetPositions(std::array<Eigen::Vector3d,4>& b_base2StanceFeet) const;

	/**
	 * user interface for CoM dynamics elements: The inertial matrix. Note this value is updated after each call of computeDerivative() method.
	 */
	Eigen::Matrix<double, 6, 6>  getM() const;

	/**
	 * user interface for CoM dynamics elements: The inverse of the inertial matrix. Note this value is updated after each call of computeDerivative() method.
	 */
	Eigen::Matrix<double, 6, 6>  getMInverse() const;

	/**
	 * user interface for CoM dynamics elements: The coriolis and centrifugal forces vector. Note this value is updated after each call of computeDerivative() method.
	 */
	Eigen::Matrix<double, 6, 1>  getC() const;

	/**
	 * user interface for CoM dynamics elements: The gravity force vector. Note this value is updated after each call of computeDerivative() method.
	 */
	Eigen::Matrix<double, 6, 1>  getG() const;


private:
	kinematic_model_t 	kinematicModel_;
	com_model_t 		comModel_;
	Eigen::Vector3d 	o_gravityVector_;
	bool constrainedIntegration_;

	std::array<bool,4> stanceLegs_;
	base_coordinate_t  q_base_;
	joint_coordinate_t qJoints_;
	joint_coordinate_t dqJoints_;

	Eigen::Vector3d com_base2CoM_;
	Eigen::Matrix<double,6,12> b_comJacobain_;
	std::array<Eigen::Vector3d,4> com_base2StanceFeet_;

	// Inertia matrix and its derivative
	Eigen::Matrix<double, 6, 6> M_;
	Eigen::Matrix<double, 6, 6> MInverse_;
	Eigen::Matrix<double, 6, 6> dMdt_;
	// Coriolis and centrifugal forces
	Eigen::Matrix<double, 6, 1> C_;
	// gravity effect on CoM in CoM coordinate
	Eigen::Matrix<double, 6, 1> MInverseG_;

};

#include "implementation/ComDynamicsBase.h"

#endif /* COMDYNAMICSBASE_H_ */
