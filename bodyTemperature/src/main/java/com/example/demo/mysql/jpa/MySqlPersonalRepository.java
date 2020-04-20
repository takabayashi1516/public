package com.example.demo.mysql.jpa;

import java.util.ArrayList;

import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;
import org.springframework.stereotype.Repository;

@Repository
public interface MySqlPersonalRepository extends JpaRepository<PersonalDataEntity, Long> {
	@Query(value = "select * from personal where " + "name = :name", nativeQuery = true)
	PersonalDataEntity get(@Param("name") String name);
	ArrayList<PersonalDataEntity> findAll();
}
