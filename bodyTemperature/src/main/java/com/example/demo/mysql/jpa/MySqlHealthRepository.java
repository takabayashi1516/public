package com.example.demo.mysql.jpa;

import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

@Repository
public interface MySqlHealthRepository extends JpaRepository<HealthDataEntity, Long> {
}
