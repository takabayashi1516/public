package com.example.demo.mysql.jpa;

import javax.persistence.criteria.CriteriaBuilder;
import javax.persistence.criteria.CriteriaQuery;
import javax.persistence.criteria.Predicate;
import javax.persistence.criteria.Root;

import org.springframework.data.jpa.domain.Specification;

public class HealthViewSpec {

	@SuppressWarnings("serial")
	public static Specification<HealthViewEntity> fieldsEquals(String field, Object value) {
		return (value == null)? null : new Specification<HealthViewEntity>() {
			@Override
			public Predicate toPredicate(Root<HealthViewEntity> root, CriteriaQuery<?> query,
					CriteriaBuilder criteriaBuilder) {
				return criteriaBuilder.equal(root.get(field), value);
			}
		};
	}

	@SuppressWarnings("serial")
	public static Specification<HealthViewEntity> fieldNumberGreaterThans(String field, Long value) {
		return (value == null)? null : new Specification<HealthViewEntity>() {
			@Override
			public Predicate toPredicate(Root<HealthViewEntity> root,
					CriteriaQuery<?> criteriaQuery, CriteriaBuilder criteriaBuilder) {
				return criteriaBuilder.greaterThan(root.get(field), value);
			}
		};
	}

	@SuppressWarnings("serial")
	public static Specification<HealthViewEntity> fieldFloatGreaterThans(String field, Float value) {
		return (value == null)? null : new Specification<HealthViewEntity>() {
			@Override
			public Predicate toPredicate(Root<HealthViewEntity> root,
					CriteriaQuery<?> criteriaQuery, CriteriaBuilder criteriaBuilder) {
				return criteriaBuilder.greaterThan(root.get(field), value);
			}
		};
	}

	@SuppressWarnings("serial")
	public static Specification<HealthViewEntity> fieldNumberLessThans(String field, Long value) {
		return (value == null)? null : new Specification<HealthViewEntity>() {
			@Override
			public Predicate toPredicate(Root<HealthViewEntity> root,
					CriteriaQuery<?> criteriaQuery, CriteriaBuilder criteriaBuilder) {
				return criteriaBuilder.lessThan(root.get(field), value);
			}
		};
	}

	@SuppressWarnings("serial")
	public static Specification<HealthViewEntity> fieldFloatLessThans(String field, Float value) {
		return (value == null)? null : new Specification<HealthViewEntity>() {
			@Override
			public Predicate toPredicate(Root<HealthViewEntity> root,
					CriteriaQuery<?> criteriaQuery, CriteriaBuilder criteriaBuilder) {
				return criteriaBuilder.lessThan(root.get(field), value);
			}
		};
	}


	@SuppressWarnings("serial")
	public static Specification<HealthViewEntity> fieldNumberGreaterThanOrEquals(String field, Long value) {
		return (value == null)? null : new Specification<HealthViewEntity>() {
			@Override
			public Predicate toPredicate(Root<HealthViewEntity> root,
					CriteriaQuery<?> criteriaQuery, CriteriaBuilder criteriaBuilder) {
				return criteriaBuilder.greaterThanOrEqualTo(root.get(field), value);
			}
		};
	}

	@SuppressWarnings("serial")
	public static Specification<HealthViewEntity> fieldFloatGreaterThanOrEquals(String field, Float value) {
		return (value == null)? null : new Specification<HealthViewEntity>() {
			@Override
			public Predicate toPredicate(Root<HealthViewEntity> root,
					CriteriaQuery<?> criteriaQuery, CriteriaBuilder criteriaBuilder) {
				return criteriaBuilder.greaterThanOrEqualTo(root.get(field), value);
			}
		};
	}

	@SuppressWarnings("serial")
	public static Specification<HealthViewEntity> fieldNumberLessThanOrEquals(String field, Long value) {
		return (value == null)? null : new Specification<HealthViewEntity>() {
			@Override
			public Predicate toPredicate(Root<HealthViewEntity> root,
					CriteriaQuery<?> criteriaQuery, CriteriaBuilder criteriaBuilder) {
				return criteriaBuilder.lessThanOrEqualTo(root.get(field), value);
			}
		};
	}

	@SuppressWarnings("serial")
	public static Specification<HealthViewEntity> fieldFloatLessThanOrEquals(String field, Float value) {
		return (value == null)? null : new Specification<HealthViewEntity>() {
			@Override
			public Predicate toPredicate(Root<HealthViewEntity> root,
					CriteriaQuery<?> criteriaQuery, CriteriaBuilder criteriaBuilder) {
				return criteriaBuilder.lessThanOrEqualTo(root.get(field), value);
			}
		};
	}
}
