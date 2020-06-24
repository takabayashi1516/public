package com.example.demo.mysql.jpa;

import javax.persistence.criteria.CriteriaBuilder;
import javax.persistence.criteria.CriteriaQuery;
import javax.persistence.criteria.Predicate;
import javax.persistence.criteria.Root;

import org.springframework.data.jpa.domain.Specification;

public class HealthSpec {

	@SuppressWarnings("serial")
	public static Specification<HealthDataEntity> fieldsEquals(String field, Object value) {
		return (value == null)? null : new Specification<HealthDataEntity>() {
			@Override
			public Predicate toPredicate(Root<HealthDataEntity> root, CriteriaQuery<?> query,
					CriteriaBuilder criteriaBuilder) {
				return criteriaBuilder.equal(root.get(field), value);
			}
		};
	}

	@SuppressWarnings("serial")
	public static Specification<HealthDataEntity> fieldNumberGreaterThans(String field, Long value) {
		return (value == null)? null : new Specification<HealthDataEntity>() {
			@Override
			public Predicate toPredicate(Root<HealthDataEntity> root,
					CriteriaQuery<?> criteriaQuery, CriteriaBuilder criteriaBuilder) {
				return criteriaBuilder.greaterThan(root.get(field), value);
			}
		};
	}

	@SuppressWarnings("serial")
	public static Specification<HealthDataEntity> fieldFloatGreaterThans(String field, Float value) {
		return (value == null)? null : new Specification<HealthDataEntity>() {
			@Override
			public Predicate toPredicate(Root<HealthDataEntity> root,
					CriteriaQuery<?> criteriaQuery, CriteriaBuilder criteriaBuilder) {
				return criteriaBuilder.greaterThan(root.get(field), value);
			}
		};
	}

	@SuppressWarnings("serial")
	public static Specification<HealthDataEntity> fieldNumberLessThans(String field, Long value) {
		return (value == null)? null : new Specification<HealthDataEntity>() {
			@Override
			public Predicate toPredicate(Root<HealthDataEntity> root,
					CriteriaQuery<?> criteriaQuery, CriteriaBuilder criteriaBuilder) {
				return criteriaBuilder.lessThan(root.get(field), value);
			}
		};
	}

	@SuppressWarnings("serial")
	public static Specification<HealthDataEntity> fieldFloatLessThans(String field, Float value) {
		return (value == null)? null : new Specification<HealthDataEntity>() {
			@Override
			public Predicate toPredicate(Root<HealthDataEntity> root,
					CriteriaQuery<?> criteriaQuery, CriteriaBuilder criteriaBuilder) {
				return criteriaBuilder.lessThan(root.get(field), value);
			}
		};
	}


	@SuppressWarnings("serial")
	public static Specification<HealthDataEntity> fieldNumberGreaterThanOrEquals(String field, Long value) {
		return (value == null)? null : new Specification<HealthDataEntity>() {
			@Override
			public Predicate toPredicate(Root<HealthDataEntity> root,
					CriteriaQuery<?> criteriaQuery, CriteriaBuilder criteriaBuilder) {
				return criteriaBuilder.greaterThanOrEqualTo(root.get(field), value);
			}
		};
	}

	@SuppressWarnings("serial")
	public static Specification<HealthDataEntity> fieldFloatGreaterThanOrEquals(String field, Float value) {
		return (value == null)? null : new Specification<HealthDataEntity>() {
			@Override
			public Predicate toPredicate(Root<HealthDataEntity> root,
					CriteriaQuery<?> criteriaQuery, CriteriaBuilder criteriaBuilder) {
				return criteriaBuilder.greaterThanOrEqualTo(root.get(field), value);
			}
		};
	}

	@SuppressWarnings("serial")
	public static Specification<HealthDataEntity> fieldNumberLessThanOrEquals(String field, Long value) {
		return (value == null)? null : new Specification<HealthDataEntity>() {
			@Override
			public Predicate toPredicate(Root<HealthDataEntity> root,
					CriteriaQuery<?> criteriaQuery, CriteriaBuilder criteriaBuilder) {
				return criteriaBuilder.lessThanOrEqualTo(root.get(field), value);
			}
		};
	}

	@SuppressWarnings("serial")
	public static Specification<HealthDataEntity> fieldFloatLessThanOrEquals(String field, Float value) {
		return (value == null)? null : new Specification<HealthDataEntity>() {
			@Override
			public Predicate toPredicate(Root<HealthDataEntity> root,
					CriteriaQuery<?> criteriaQuery, CriteriaBuilder criteriaBuilder) {
				return criteriaBuilder.lessThanOrEqualTo(root.get(field), value);
			}
		};
	}
}
