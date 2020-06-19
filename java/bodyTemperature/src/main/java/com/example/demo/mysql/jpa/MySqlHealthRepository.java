package com.example.demo.mysql.jpa;

import java.util.List;

import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;
import org.springframework.stereotype.Repository;

@Repository
public interface MySqlHealthRepository extends JpaRepository<HealthDataEntity, Long> {
	List<HealthDataEntity> findAll();
	@Query(value = "select * from health where id = :id", nativeQuery = true)
	HealthDataEntity get(@Param("id") long id);
	@Query(value = "select * from health where personal_id = :person and timestamp >= :ts_start and timestamp <= :ts_end", nativeQuery = true)
	List<HealthDataEntity> get(@Param("person") long person,
			@Param("ts_start") long ts_start, @Param("ts_end") long ts_end);
	@Query(value = "select * from health where personal_id = :person order by id desc limit :num", nativeQuery = true)
	List<HealthDataEntity> getLatest(@Param("person") long person, @Param("num") int num);

}
