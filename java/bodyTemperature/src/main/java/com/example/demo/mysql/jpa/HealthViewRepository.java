package com.example.demo.mysql.jpa;

import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.JpaSpecificationExecutor;
import org.springframework.stereotype.Repository;

@Repository
public interface HealthViewRepository extends JpaRepository<HealthViewEntity, Long>,
		JpaSpecificationExecutor<HealthViewEntity> {
//	List<HealthViewEntity> findAll();
}
