package com.example.demo.mysql.jpa;

import java.util.ArrayList;
import java.util.List;

import org.springframework.context.annotation.Profile;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;
import org.springframework.stereotype.Repository;

@Repository
public interface MySqlHealthRepository extends JpaRepository<HealthDataEntity, Long> {
//	@Query(value = "select * from " + ":table", nativeQuery = true)
//	ArrayList<HealthDataEntity> getAll(@Param("table") Object table);

//	@Query(value = ":query", nativeQuery = true)
//	List<DataEntity> doQuery(@Param("query") String query);
}
