package com.example.demo.mysql.jpa;

import java.util.ArrayList;

import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;
import org.springframework.stereotype.Repository;

@Repository
public interface MySqlHealthViewRepository extends JpaRepository<HealthViewEntity, Long> {
	ArrayList<HealthViewEntity> findAll();
	@Query(value = "select * from health_view where person = :person", nativeQuery = true)
	ArrayList<HealthViewEntity> get(@Param("person") long person);
	@Query(value = "select * from health_view where name = :name", nativeQuery = true)
	ArrayList<HealthViewEntity> get(@Param("name") String name);
}
